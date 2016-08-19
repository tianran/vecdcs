#include "VecDCS_Model.h"

#include <fstream>
#include <cmath>
#include "Vocab_Aux.h"
#include "DAO.h"
#include "Matrix_Aux.h"
#include "Fibonacci_Heap.h"

using namespace std;
using namespace Eigen;

VecDCS_Model::VecDCS_Model(const string& vocab_fn, const string& role_fn, const string& model_fn) {

    const long nw = Vocab_Aux::init_vocab(vocab_fn, word_map, vec_word); //Number of words
    const long nr = Vocab_Aux::init_vocab(role_fn, role_map, vec_role); //Number of roles

    ifstream model_file(model_fn);
    dim = DAO::read_long(model_file); //Vector dimension
    n_neg = DAO::read_int(model_file); //Negative samples
    we = DAO::read_float(model_file); //Initial learning rate for word vectors
    wl = DAO::read_float(model_file); //Decaying rate for word vectors
    re = DAO::read_float(model_file); //Initial learning rate for role matrices
    rl = DAO::read_float(model_file); //Decaying rate for role matrices
    rk = DAO::read_float(model_file); //Regularizer for isotropy
    rg = DAO::read_float(model_file); //Regularizer for reversibility

    c_vecs.resize(dim, nw);
    Matrix_Aux::read_matrix(model_file, c_vecs);
    t_vecs.resize(nw, dim);
    Matrix_Aux::read_matrix(model_file, t_vecs);
    role_mats.resize(nr);
    inv_mats.resize(nr);
    for (auto& x : role_mats) {
        x.resize(dim, dim);
        Matrix_Aux::read_matrix(model_file, x);
    }
    for (auto& x : inv_mats) {
        x.resize(dim, dim);
        Matrix_Aux::read_matrix(model_file, x);
    }
    model_file.close();

    c_vecs.colwise().normalize();
    t_vecs.rowwise().normalize();
    for (auto& x : role_mats) {
        x /= sqrt(x.squaredNorm() / dim);
    }
    for (auto& x : inv_mats) {
        x /= sqrt(x.squaredNorm() / dim);
    }
}

long VecDCS_Model::dimension() const {
    return dim;
}

void VecDCS_Model::set_query_vec(const string& word, RowVectorXf& vec) const {
    vec = t_vecs.row(Vocab_Aux::get_index_unk_type(word_map, word));
}

void VecDCS_Model::set_answer_vec(const string& word, VectorXf& vec) const {
    vec = c_vecs.col(Vocab_Aux::get_index_unk_type(word_map, word));
}

void VecDCS_Model::trans_query_vec(const RowVectorXf& from, const string& proj_role, const string& inv_role,
                     RowVectorXf& to) const {
    long proj_i = Vocab_Aux::get_index_unk(role_map, proj_role);
    long inv_i = Vocab_Aux::get_index_unk(role_map, inv_role);
    to = from * role_mats[proj_i] * inv_mats[inv_i];
}

vector<pair<float, string>> VecDCS_Model::max_k(const float *begin, const float *end, int k) const {
    Fibonacci_Heap<long> knn_heap;
    long i = 0;
    for (auto x = begin; x != end; ++x) {
        knn_heap.insert(i, *x);
        if (knn_heap.size() > k) knn_heap.remove_min();
        ++i;
    }
    vector<pair<float, string>> ret(knn_heap.size());
    for (auto x = ret.rbegin(); x != ret.rend(); ++x) {
        float tmp = knn_heap.min_key();
        *x = make_pair(tmp, vec_word[knn_heap.remove_min()]);
    }
    return ret;
}

vector<pair<float, string>> VecDCS_Model::top_similar(const RowVectorXf& vec, int k) const {
    VectorXf scores = t_vecs * vec.transpose();
    return max_k(scores.data(), scores.data() + scores.size(), k);
}

vector<pair<float, string>> VecDCS_Model::top_answers(const RowVectorXf& vec, int k) const {
    RowVectorXf scores = vec * c_vecs;
    return max_k(scores.data(), scores.data() + scores.size(), k);
}
