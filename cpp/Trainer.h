#ifndef VECDCS_TRAINER_H
#define VECDCS_TRAINER_H

#include <string>
#include <vector>
#include <mutex>
#include <atomic>
#include <unordered_map>
#include <iostream>

#include "Eigen/Core"

#include "Weight_Sampler.h"

class Trainer {

    std::vector<std::string> file_names;
    int next_file_index;
    int trained_pass;
    int goal_pass;
    std::mutex file_name_lock;

    std::unordered_map<std::string, long> word_map;
    std::unordered_map<std::string, long> role_map;

    const long dim;
    const int num_neg;

    Weight_Sampler nword_samp;
    Weight_Sampler nrole_samp;

    Eigen::MatrixXf c_vecs;
    Eigen::MatrixXf t_vecs;
    std::vector<Eigen::MatrixXf> role_matrices;
    std::vector<Eigen::MatrixXf> inv_matrices;

    std::atomic<unsigned long long>* c_steps;
    std::atomic<unsigned long long>* t_steps;
    const float w_eta;
    const float w_lambda;

    std::atomic<unsigned long long>* r_steps;
    const float r_eta;
    const float r_lambda;
    const float r_kappa;
    const float r_gamma;

    std::string train_out_fn;
    void train_para(long long seed);

public:

    Trainer(const std::vector<std::string>& fns, const std::string& vocab_fn, const std::string& role_fn,
            long dim, int n_neg, float we, float wl, float re, float rl, float rk, float rg);

    Trainer(const std::vector<std::string>& fns, const std::string& vocab_fn, const std::string& role_fn,
            std::istream&& model_file);

    ~Trainer();

    void save_model(const std::string& model_fn);

    void train(int epoch, int para, const std::string& out_fn);
};

#endif //VECDCS_TRAINER_H
