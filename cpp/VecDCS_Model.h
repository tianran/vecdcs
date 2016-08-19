#ifndef VECDCS_CPP_VECDCS_MODEL_H
#define VECDCS_CPP_VECDCS_MODEL_H

#include <unordered_map>
#include <string>
#include <vector>
#include <utility>
#include "Eigen/Core"

class VecDCS_Model {

protected:
    long dim;
    int n_neg;
    float we;
    float wl;
    float re;
    float rl;
    float rk;
    float rg;

    std::unordered_map<std::string, long> word_map;
    std::vector<std::string> vec_word;

    std::unordered_map<std::string, long> role_map;
    std::vector<std::string> vec_role;

    Eigen::MatrixXf c_vecs;
    Eigen::MatrixXf t_vecs;
    std::vector<Eigen::MatrixXf> role_mats;
    std::vector<Eigen::MatrixXf> inv_mats;

    std::vector<std::pair<float, std::string>> max_k(const float* begin, const float* end, int k) const;

public:
    VecDCS_Model() = default;
    VecDCS_Model(const std::string& vocab_fn, const std::string& role_fn, const std::string& model_fn);

    long dimension() const;

    void set_query_vec(const std::string& word, Eigen::RowVectorXf& vec) const;
    void trans_query_vec(const Eigen::RowVectorXf& from, const std::string& proj_role, const std::string& inv_role,
                         Eigen::RowVectorXf& to) const;
    void set_answer_vec(const std::string& word, Eigen::VectorXf& vec) const;

    std::vector<std::pair<float, std::string>> top_similar(const Eigen::RowVectorXf& vec, int k) const;
    std::vector<std::pair<float, std::string>> top_answers(const Eigen::RowVectorXf& vec, int k) const;
};

#endif //VECDCS_CPP_VECDCS_MODEL_H
