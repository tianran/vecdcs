#ifndef TRAINER_AUX_H
#define TRAINER_AUX_H

#include <iostream>
#include <vector>
#include <atomic>

#include "Eigen/Core"

namespace Trainer_Aux {

    float sig_k(float x, int k);

    std::atomic<unsigned long long>* init_vector_atomic(long sz, std::istream& is);

    void clear_cache(std::vector<Eigen::MatrixXf>& cache);

    void flush_accum(std::vector<Eigen::MatrixXf>& accum, std::vector<Eigen::MatrixXf>& genuine);
}

#endif //TRAINER_AUX_H
