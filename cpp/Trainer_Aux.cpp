#include "Trainer_Aux.h"

#include <cmath>
#include "Eigen/Core"
#include "DAO.h"

using namespace std;
using namespace Eigen;

namespace Trainer_Aux {

    float sig_k(float x, int k) {
        return k / (exp(x) + k);
    }

    atomic<unsigned long long>* init_vector_atomic(long sz, istream& is) {
        atomic<unsigned long long>* ret = new atomic<unsigned long long>[sz]();
        for (long i = 0; i != sz; ++i) {
            ret[i] = DAO::read_long(is);
        }
        return ret;
    }

    void clear_cache(vector<MatrixXf>& cache) {
        for (auto& x : cache) if (x.size() > 0) x.resize(0, 0);
    }

    void flush_accum(vector<MatrixXf>& accum, vector<MatrixXf>& genuine) {
        for (long i = 0; i != accum.size(); ++i) {
            if (accum[i].size() > 0) {
                genuine[i] += accum[i];
                accum[i].resize(0, 0);
            }
        }
    }
}
