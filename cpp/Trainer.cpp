#include "Trainer.h"

#include <cassert>
#include <thread>
#include <chrono>
#include <random>
#include <algorithm>
#include <cmath>
#include <map>

#include "Random_Generator.h"
#include "Simple_DCS_Node.h"
#include "DCS_Tree_Reader.h"
#include "DAO.h"
#include "Matrix_Aux.h"
#include "Vocab_Aux.h"
#include "Trainer_Aux.h"

#ifndef PASS_PER_EPOCH
 #define PASS_PER_EPOCH 8
#endif //PASS_PER_EPOCH

#ifndef CALC_REGULARIZER_UPDATES
 #define CALC_REGULARIZER_UPDATES 1024
#endif //CALC_REGULARIZER_UPDATES

#ifndef LOCAL_UPDATES
 #define LOCAL_UPDATES 1024
#endif //LOCAL_UPDATES

#ifndef LOCAL_VEC_UPDATES
 #define LOCAL_VEC_UPDATES 16
#endif //LOCAL_VEC_UPDATES

using namespace std;
using namespace Eigen;

Trainer::Trainer(const vector<string> &fns, const string &vocab_fn, const string &role_fn, 
                 long d, int n_neg, float we, float wl, float re, float rl, float rk, float rg)
        : file_names(fns), dim(d), num_neg(n_neg), w_eta(we), w_lambda(wl), r_eta(re), r_lambda(rl), r_kappa(rk), r_gamma(rg) {

    const long nw = Vocab_Aux::init_vocab(vocab_fn, word_map, nword_samp);
    const long nr = Vocab_Aux::init_vocab(role_fn, role_map, nrole_samp);

    Random_Generator rand(chrono::system_clock::now().time_since_epoch().count());
    normal_distribution<float> gaussian(0.0f, 1.0f / sqrt(dim));

    c_vecs.resize(dim, nw);
    generate(c_vecs.data(), c_vecs.data() + c_vecs.size(), [&](){ return gaussian(rand); });
    t_vecs.resize(nw, dim);
    generate(t_vecs.data(), t_vecs.data() + t_vecs.size(), [&](){ return gaussian(rand); });

    role_matrices.resize(nr);
    inv_matrices.reserve(nr);
    for (auto& x: role_matrices) {
        x.resize(dim, dim);
        generate(x.data(), x.data() + x.size(), [&](){ return gaussian(rand); });
        x = (x + MatrixXf::Identity(dim, dim)) * 0.5;
        inv_matrices.push_back(x.transpose());
    }

    c_steps = new atomic<unsigned long long>[nw]();
    t_steps = new atomic<unsigned long long>[nw]();
    r_steps = new atomic<unsigned long long>[nr]();

    Eigen::initParallel();
}

Trainer::Trainer(const vector<string> &fns, const string &vocab_fn, const string &role_fn, istream &&model_file)
        : file_names(fns), dim(DAO::read_long(model_file)), num_neg(DAO::read_int(model_file)),
          w_eta(DAO::read_float(model_file)), w_lambda(DAO::read_float(model_file)), r_eta(DAO::read_float(model_file)),
          r_lambda(DAO::read_float(model_file)), r_kappa(DAO::read_float(model_file)), r_gamma(DAO::read_float(model_file)) {

    const long nw = Vocab_Aux::init_vocab(vocab_fn, word_map, nword_samp);
    const long nr = Vocab_Aux::init_vocab(role_fn, role_map, nrole_samp);

    c_vecs.resize(dim, nw);
    Matrix_Aux::read_matrix(model_file, c_vecs);
    t_vecs.resize(nw, dim);
    Matrix_Aux::read_matrix(model_file, t_vecs);

    role_matrices.resize(nr);
    for (auto& x: role_matrices) {
        x.resize(dim, dim);
        Matrix_Aux::read_matrix(model_file, x);
    }
    inv_matrices.resize(nr);
    for (auto& x: inv_matrices) {
        x.resize(dim, dim);
        Matrix_Aux::read_matrix(model_file, x);
    }

    c_steps = Trainer_Aux::init_vector_atomic(nw, model_file);
    t_steps = Trainer_Aux::init_vector_atomic(nw, model_file);
    r_steps = Trainer_Aux::init_vector_atomic(nr, model_file);

    Eigen::initParallel();
}

Trainer::~Trainer() {
    delete[] c_steps;
    delete[] t_steps;
    delete[] r_steps;
}

void Trainer::save_model(const string &model_fn) {
    ofstream ofs(model_fn);
    DAO::write_long(ofs, dim);
    DAO::write_int(ofs, num_neg);
    DAO::write_float(ofs, w_eta);
    DAO::write_float(ofs, w_lambda);
    DAO::write_float(ofs, r_eta);
    DAO::write_float(ofs, r_lambda);
    DAO::write_float(ofs, r_kappa);
    DAO::write_float(ofs, r_gamma);
    Matrix_Aux::write_matrix(ofs, c_vecs);
    Matrix_Aux::write_matrix(ofs, t_vecs);
    for (const auto& x: role_matrices) Matrix_Aux::write_matrix(ofs, x);
    for (const auto& x: inv_matrices) Matrix_Aux::write_matrix(ofs, x);
    for (long i = 0; i != c_vecs.cols(); ++i) {
        long tmp = c_steps[i];
        DAO::write_long(ofs, tmp);
    }
    for (long i = 0; i != t_vecs.rows(); ++i) {
        long tmp = t_steps[i];
        DAO::write_long(ofs, tmp);
    }
    for (long i = 0; i != role_matrices.size(); ++i) {
        long tmp = r_steps[i];
        DAO::write_long(ofs, tmp);
    }
}

void Trainer::train_para(long long seed) {

    vector<MatrixXf> cache_role_matrices(role_matrices.size());
    vector<MatrixXf> accum_role_matrices(role_matrices.size());
    vector<MatrixXf> cache_inv_matrices(inv_matrices.size());
    vector<MatrixXf> accum_inv_matrices(inv_matrices.size());
    int update_counter = 0;

    map<long, RowVectorXf> cache_t_vecs;
    map<long, RowVectorXf> accum_t_vecs;
    map<long, VectorXf> cache_c_vecs;
    map<long, VectorXf> accum_c_vecs;
    int update_vec_counter = 0;

    Random_Generator rand(seed);

    while(true) {
        file_name_lock.lock();
        if (trained_pass == goal_pass) {
            file_name_lock.unlock();
            break;
        }
        if (next_file_index == 0) random_shuffle(file_names.begin(), file_names.end(), rand);
        string fn = file_names[next_file_index];
        if (++next_file_index == file_names.size()) {
            next_file_index = 0;
            ++trained_pass;
        }
        ofstream out_file(train_out_fn, ios_base::app);
        out_file << fn << endl;
        out_file.close();
        file_name_lock.unlock();

        assert(!t_vecs.hasNaN());
        assert(!c_vecs.hasNaN());
        for (const auto& x : role_matrices) assert(!x.hasNaN());
        for (const auto& x : inv_matrices) assert(!x.hasNaN());

        ifstream file(fn);
        string line;
        while(getline(file, line)) {
            vector<Simple_DCS_Node> tree = read_dcs_tree<Simple_DCS_Node>(file, stoi(line));
            for (const auto& node : tree) {
                double walks = node.word_weight() / PASS_PER_EPOCH;
                while(walks >= 1.0 || (rand() & 0xFFFFFFFFFFFFFULL) / double(0x10000000000000ULL) < walks) {
                    auto samp_path = node.sample_path(rand);
                    int pl = samp_path.first.size();
                    long t_i = Vocab_Aux::get_index_unk_type(word_map, node.word);
                    long c_i = Vocab_Aux::get_index_unk_type(word_map, samp_path.second);
                    vector<long> n_i(num_neg);
                    for (auto& x : n_i) x = nword_samp(rand);

                    // take vectors
                    float t_scale = 1.0f / (1.0f + w_eta * w_lambda * t_steps[t_i]++);
                    RowVectorXf& cached_t_vec = cache_t_vecs[t_i];
                    if (cached_t_vec.size() == 0) {
                        cached_t_vec = t_vecs.row(t_i);
                        accum_t_vecs[t_i] = RowVectorXf::Zero(dim);
                    }
                    RowVectorXf orig_t_vec = cached_t_vec;
                    float c_scale = 1.0f / (1.0f + w_eta * w_lambda / (num_neg + 1) * c_steps[c_i]++);
                    VectorXf& cached_c_vec = cache_c_vecs[c_i];
                    if (cached_c_vec.size() == 0) {
                        cached_c_vec = c_vecs.col(c_i);
                        accum_c_vecs[c_i] = VectorXf::Zero(dim);
                    }
                    VectorXf orig_c_vec = cached_c_vec;
                    MatrixXf orig_neg(dim, num_neg);
                    vector<float> neg_scale(num_neg);
                    for (int i = 0; i != num_neg; ++i) {
                        neg_scale[i] = 1.0f / (1.0f + w_eta * w_lambda / (num_neg + 1) * c_steps[n_i[i]]++);
                        long n_i_i = n_i[i];
                        VectorXf& cached_neg = cache_c_vecs[n_i_i];
                        if (cached_neg.size() == 0) {
                            cached_neg = c_vecs.col(n_i_i);
                            accum_c_vecs[n_i_i] = VectorXf::Zero(dim);
                        }
                        orig_neg.col(i) = cached_neg;
                    }

                    // take role matrices
                    int n_bd = rand() % (pl - 1);
                    vector<MatrixXf*> r_mats(pl);
                    vector<vector<MatrixXf*>> neg_r_mats(num_neg);
                    MatrixXf* t_r_accum;
                    MatrixXf* c_r_accum;
                    vector<MatrixXf*> neg_r_accum(num_neg);
                    float t_r_scale;
                    float c_r_scale;
                    vector<float> neg_r_scale(num_neg);

                    auto scale_cache_matrix_regularize = [&](int path_i, long role_i, MatrixXf*& to_focus, MatrixXf** to_accum, int incr) {
                        float ret_scale;
                        MatrixXf* mat;
                        MatrixXf* inv_mat;
                        MatrixXf* m_accum;
                        MatrixXf* inv_accum;
                        if (path_i & 1) {
                            if (cache_inv_matrices[role_i].size() == 0) cache_inv_matrices[role_i] = inv_matrices[role_i];
                            to_focus = &cache_inv_matrices[role_i];
                            if (to_accum) {
                                if (accum_inv_matrices[role_i].size() == 0) accum_inv_matrices[role_i] = MatrixXf::Zero(dim, dim);
                                *to_accum = &accum_inv_matrices[role_i];
                                unsigned long long r_cur = r_steps[role_i]++;
                                bool hit = r_cur % (CALC_REGULARIZER_UPDATES * 2 * (1 + num_neg)) == 0;
                                for (int i = 0; i != incr; ++i) if (r_steps[role_i]++ % (CALC_REGULARIZER_UPDATES * 2 * (1 + num_neg)) == 0) hit = true;
                                ret_scale = 1.0f / (1.0f + r_lambda / (2 * (1 + num_neg)) * r_eta * r_cur);
                                if (hit) {
                                    inv_mat = to_focus;
                                    inv_accum = *to_accum;
                                    if (cache_role_matrices[role_i].size() == 0) cache_role_matrices[role_i] = role_matrices[role_i];
                                    mat = &cache_role_matrices[role_i];
                                    if (accum_role_matrices[role_i].size() == 0) accum_role_matrices[role_i] = MatrixXf::Zero(dim, dim);
                                    m_accum = &accum_role_matrices[role_i];
                                } else {
                                    return ret_scale;
                                }
                            } else {
                                return 1.0f;
                            }
                        } else {
                            if (cache_role_matrices[role_i].size() == 0) cache_role_matrices[role_i] = role_matrices[role_i];
                            to_focus = &cache_role_matrices[role_i];
                            if (to_accum) {
                                if (accum_role_matrices[role_i].size() == 0) accum_role_matrices[role_i] = MatrixXf::Zero(dim, dim);
                                *to_accum = &accum_role_matrices[role_i];
                                unsigned long long r_cur = r_steps[role_i]++;
                                bool hit = r_cur % (CALC_REGULARIZER_UPDATES * 2 * (1 + num_neg)) == 0;
                                for (int i = 0; i != incr; ++i) if (r_steps[role_i]++ % (CALC_REGULARIZER_UPDATES * 2 * (1 + num_neg)) == 0) hit = true;
                                ret_scale = 1.0f / (1.0f + r_lambda / (2 * (1 + num_neg)) * r_eta * r_cur);
                                if (hit) {
                                    mat = to_focus;
                                    m_accum = *to_accum;
                                    if (cache_inv_matrices[role_i].size() == 0) cache_inv_matrices[role_i] = inv_matrices[role_i];
                                    inv_mat = &cache_inv_matrices[role_i];
                                    if (accum_inv_matrices[role_i].size() == 0) accum_inv_matrices[role_i] = MatrixXf::Zero(dim, dim);
                                    inv_accum = &accum_inv_matrices[role_i];
                                } else {
                                    return ret_scale;
                                }
                            } else {
                                return 1.0f;
                            }
                        }

                        MatrixXf xx = *inv_mat * (*mat) * (CALC_REGULARIZER_UPDATES * 2 * (1 + num_neg) * r_gamma * r_eta * ret_scale * ret_scale);
                        xx -= (xx.trace() / dim) * MatrixXf::Identity(dim, dim);

                        MatrixXf m_regu = *mat * mat->transpose() * (CALC_REGULARIZER_UPDATES * 2 * (1 + num_neg) * r_kappa * r_eta * ret_scale * ret_scale);
                        m_regu -= (m_regu.trace() / dim) * MatrixXf::Identity(dim, dim);
                        m_regu = (m_regu * (*mat) + inv_mat->transpose() * xx) * ret_scale;

                        MatrixXf inv_regu = *inv_mat * inv_mat->transpose() * (CALC_REGULARIZER_UPDATES * 2 * (1 + num_neg) * r_kappa * r_eta * ret_scale * ret_scale);
                        inv_regu -= (inv_regu.trace() / dim) * MatrixXf::Identity(dim, dim);
                        inv_regu = (inv_regu * (*inv_mat) + xx * mat->transpose()) * ret_scale;

                        *mat -= m_regu;
                        *m_accum -= m_regu;
                        *inv_mat -= inv_regu;
                        *inv_accum -= inv_regu;

                        return ret_scale;
                    };
                    auto path_pstr = samp_path.first.begin();
                    for (int i = 0; i != n_bd; ++i, ++path_pstr) {
                        scale_cache_matrix_regularize(i, Vocab_Aux::get_index_unk(role_map, *path_pstr), r_mats[i], nullptr, 0);
                    }
                    long t_r_i = Vocab_Aux::get_index_unk(role_map, *path_pstr);
                    t_r_scale = scale_cache_matrix_regularize(n_bd, t_r_i, r_mats[n_bd], &t_r_accum, num_neg);
                    ++path_pstr;
                    long c_r_i = Vocab_Aux::get_index_unk(role_map, *path_pstr);
                    c_r_scale = scale_cache_matrix_regularize(n_bd + 1, c_r_i, r_mats[n_bd + 1], &c_r_accum, 0);
                    for (int i = 0; i != num_neg; ++i) {
                        neg_r_mats[i].resize(pl - n_bd - 1);
                        neg_r_scale[i] = scale_cache_matrix_regularize(n_bd + 1, nrole_samp(rand), neg_r_mats[i][0], &neg_r_accum[i], 0);
                    }
                    ++path_pstr;
                    for (int i = n_bd + 2; i != pl; ++i, ++path_pstr) {
                        scale_cache_matrix_regularize(i, Vocab_Aux::get_index_unk(role_map, *path_pstr), r_mats[i], nullptr, 0);
                        for (auto& x: neg_r_mats) {
                            scale_cache_matrix_regularize(i, nrole_samp(rand), x[i - n_bd - 1], nullptr, 0);
                        }
                    }

                    // calc
                    RowVectorXf left_t_r;
                    RowVectorXf right_t_r;
                    vector<RowVectorXf> t_final_neg(num_neg);
                    RowVectorXf t_final_c = orig_t_vec.normalized();
                    for (int i = 0; i != n_bd; ++i) {
                        t_final_c = (t_final_c * (*r_mats[i])) * sqrt(dim / r_mats[i]->squaredNorm());
                    }
                    left_t_r = t_final_c;
                    t_final_c = (t_final_c * (*r_mats[n_bd])) * sqrt(dim / r_mats[n_bd]->squaredNorm());
                    right_t_r = t_final_c;
                    for (auto& x: t_final_neg) x = t_final_c;
                    for (int i = n_bd + 1; i != pl; ++i) {
                        t_final_c = (t_final_c * (*r_mats[i])) * sqrt(dim / r_mats[i]->squaredNorm());
                    }
                    for (int i = 0; i != num_neg; ++i) {
                        for (auto x: neg_r_mats[i]) t_final_neg[i] = (t_final_neg[i] * (*x)) * sqrt(dim / x->squaredNorm());
                    }

                    VectorXf right_c_r;
                    VectorXf left_c_r;
                    VectorXf c_final_t = orig_c_vec.normalized();
                    for (int i = pl - 1; i != n_bd + 1; --i) {
                        c_final_t = ((*r_mats[i]) * c_final_t) * sqrt(dim / r_mats[i]->squaredNorm());
                    }
                    right_c_r = c_final_t;
                    c_final_t = ((*r_mats[n_bd + 1]) * c_final_t) * sqrt(dim / r_mats[n_bd + 1]->squaredNorm());
                    left_c_r = c_final_t;

                    MatrixXf right_neg_r;
                    MatrixXf left_neg_r;
                    MatrixXf neg_final_t = orig_neg.colwise().normalized();
                    for (int i = pl - n_bd - 2; i != 0; --i) {
                        for (int j = 0; j != num_neg; ++j) {
                            neg_final_t.col(j) = ((*neg_r_mats[j][i]) * neg_final_t.col(j)) * sqrt(dim / neg_r_mats[j][i]->squaredNorm());
                        }
                    }
                    right_neg_r = neg_final_t;
                    for (int i = 0; i != num_neg; ++i) {
                        neg_final_t.col(i) = ((*neg_r_mats[i][0]) * neg_final_t.col(i)) * sqrt(dim / neg_r_mats[i][0]->squaredNorm());
                    }
                    left_neg_r = neg_final_t;

                    for (int i = n_bd; i >= 0; --i) {
                        float tmp = sqrt(dim / r_mats[i]->squaredNorm());
                        c_final_t = ((*r_mats[i]) * c_final_t) * tmp;
                        neg_final_t = ((*r_mats[i]) * neg_final_t) * tmp;
                    }

                    float tmp_norm;

                    float t_alpha = Trainer_Aux::sig_k((orig_t_vec * c_final_t)(0, 0) * t_scale, num_neg);
                    if ((tmp_norm = c_final_t.norm()) > 2.0f) c_final_t *= 2.0f / tmp_norm;
                    RowVectorXf t_alpha_neg = orig_t_vec * neg_final_t * t_scale;
                    for (int i = 0; i != num_neg; ++i) {
                        t_alpha_neg(i) = Trainer_Aux::sig_k(t_alpha_neg(i), num_neg) - 1.0f;
                        if ((tmp_norm = neg_final_t.col(i).norm()) > 2.0f) neg_final_t.col(i) *= 2.0f / tmp_norm;
                    }
                    RowVectorXf t_update = (c_final_t * t_alpha + (neg_final_t * t_alpha_neg.transpose()).rowwise().sum()) * w_eta;
                    accum_t_vecs[t_i] += t_update;

                    c_scale = Trainer_Aux::sig_k((t_final_c * orig_c_vec)(0, 0) * c_scale, num_neg) * w_eta;
                    if ((tmp_norm = t_final_c.norm()) > 2.0f) t_final_c *= 2.0f / tmp_norm;
                    for (int i = 0; i != num_neg; ++i) {
                        neg_scale[i] = (Trainer_Aux::sig_k((t_final_neg[i] * orig_neg.col(i))(0, 0) * neg_scale[i], num_neg) - 1.0f) * w_eta;
                        if ((tmp_norm = t_final_neg[i].norm()) > 2.0f) t_final_neg[i] *= 2.0f / tmp_norm;
                    }
                    VectorXf tmp_update = t_final_c.transpose() * c_scale;
                    accum_c_vecs[c_i] += tmp_update;
                    for (int i = 0; i != num_neg; ++i) {
                        tmp_update = t_final_neg[i].transpose() * neg_scale[i];
                        accum_c_vecs[n_i[i]] += tmp_update;
                    }

                    RowVectorXf left_t_r_trans = left_t_r * (*r_mats[n_bd]);
                    if ((tmp_norm = left_t_r.norm()) > 2.0f) left_t_r *= 2.0f / tmp_norm;
                    float t_r_alpha = Trainer_Aux::sig_k((left_t_r_trans * left_c_r)(0, 0) * t_r_scale, num_neg);
                    if ((tmp_norm = left_c_r.norm()) > 2.0f) left_c_r *= 2.0f / tmp_norm;
                    RowVectorXf t_r_alpha_neg = left_t_r_trans * left_neg_r * t_r_scale;
                    for (int i = 0; i != num_neg; ++i) {
                        t_r_alpha_neg(i) = Trainer_Aux::sig_k(t_r_alpha_neg(i), num_neg) - 1.0f;
                        if ((tmp_norm = left_neg_r.col(i).norm()) > 2.0f) left_neg_r.col(i) *= 2.0f / tmp_norm;
                    }
                    MatrixXf t_r_update = r_eta * left_t_r.transpose() *
                            (left_c_r * t_r_alpha + (left_neg_r * t_r_alpha_neg.transpose()).rowwise().sum()).transpose();

                    c_r_scale = Trainer_Aux::sig_k((right_t_r * (*r_mats[n_bd + 1]) * right_c_r)(0, 0) * c_r_scale, num_neg) * r_eta;
                    if ((tmp_norm = right_c_r.norm()) > 2.0f) right_c_r *= 2.0f / tmp_norm;
                    for (int i = 0; i != num_neg; ++i) {
                        neg_r_scale[i] = (Trainer_Aux::sig_k((right_t_r * (*neg_r_mats[i][0]) * right_neg_r.col(i))(0, 0) * neg_r_scale[i], num_neg) - 1.0f) * r_eta;
                        if ((tmp_norm = right_neg_r.col(i).norm()) > 2.0f) right_neg_r.col(i) *= 2.0f / tmp_norm;
                    }
                    if ((tmp_norm = right_t_r.norm()) > 2.0f) right_t_r *= 2.0f / tmp_norm;

                    // update
                    *t_r_accum += t_r_update;
                    *(r_mats[n_bd]) += t_r_update;

                    MatrixXf tmp_r_update = c_r_scale * right_t_r.transpose() * right_c_r.transpose();
                    *c_r_accum += tmp_r_update;
                    *(r_mats[n_bd + 1]) += tmp_r_update;
                    for (int i = 0; i != num_neg; ++i) {
                        tmp_r_update = neg_r_scale[i] * right_t_r.transpose() * right_neg_r.col(i).transpose();
                        *neg_r_accum[i] += tmp_r_update;
                        *(neg_r_mats[i][0]) += tmp_r_update;
                    }

                    ++update_counter;
                    if (update_counter == LOCAL_UPDATES) {
                        Trainer_Aux::flush_accum(accum_role_matrices, role_matrices);
                        Trainer_Aux::flush_accum(accum_inv_matrices, inv_matrices);
                        Trainer_Aux::clear_cache(cache_role_matrices);
                        Trainer_Aux::clear_cache(cache_inv_matrices);
                        update_counter = 0;
                    }
                    ++update_vec_counter;
                    if (update_vec_counter == LOCAL_VEC_UPDATES) {
                        for (auto& x : accum_t_vecs) t_vecs.row(x.first) += x.second;
                        accum_t_vecs.clear();
                        for (auto& x : accum_c_vecs) c_vecs.col(x.first) += x.second;
                        accum_c_vecs.clear();
                        cache_t_vecs.clear();
                        cache_c_vecs.clear();
                        update_vec_counter = 0;
                    }

                    walks -= 1.0;
                }
            }
        }
    }

    Trainer_Aux::flush_accum(accum_role_matrices, role_matrices);
    Trainer_Aux::flush_accum(accum_inv_matrices, inv_matrices);
    for (auto& x : accum_t_vecs) t_vecs.row(x.first) += x.second;
    accum_t_vecs.clear();
    for (auto& x : accum_c_vecs) c_vecs.col(x.first) += x.second;
    accum_c_vecs.clear();
}

void Trainer::train(int epoch, int para, const string& out_fn) {

    next_file_index = 0;
    trained_pass = 0;
    goal_pass = PASS_PER_EPOCH * epoch;

    train_out_fn = out_fn;

    const long long seed = chrono::system_clock::now().time_since_epoch().count();

    vector<thread> threads;
    threads.reserve(para);
    for (int i = 0; i != para; ++i) {
        threads.emplace_back(&Trainer::train_para, this, seed + i);
    }
    for (auto& x : threads) x.join();

    save_model(out_fn);
}