#ifndef __SPEARMANS_RHO_H
#define __SPEARMANS_RHO_H

#include <vector>
#include <algorithm>
#include <cassert>

template <class T>
void convert2ranks(T begin, T end) {

    std::vector<T> tosort;
    for (T i = begin; i != end; ++i) {
        tosort.push_back(i);
    }
    std::stable_sort(tosort.begin(), tosort.end(), [] (T a, T b) { return *a < *b; });

    auto oldv = *tosort[0];
    long tmpcounter = 1;
    for (long i = 1; i != tosort.size(); ++i) {
        auto v = *tosort[i];
        if (v > oldv) {
            float avrrank = (i * 2 - tmpcounter - 1) * 0.5f;
            for (long j = i - tmpcounter; j < i; j++) {
                *tosort[j] = avrrank;
            }
            oldv = v;
            tmpcounter = 1;
        } else {
            tmpcounter++;
        }
    }
    float avrrank = (tosort.size() * 2 - tmpcounter - 1) * 0.5f;
    for (long j = tosort.size() - tmpcounter; j != tosort.size(); ++j) {
        *tosort[j] = avrrank;
    }
}

inline float spearmans_rho(std::vector<float>& a, std::vector<float>& b) {
    assert(a.size() == b.size());
    assert(!a.empty());
    convert2ranks(a.begin(), a.end());
    convert2ranks(b.begin(), b.end());

    float rho = 0.0f;
    for (long i = 0; i < a.size(); i++) {
        float tmp = a[i] - b[i];
        rho += tmp * tmp;
    }
    float n = a.size();
    rho = 1.0f - rho * 6.0f / (n * (n * n - 1.0f));

    return rho;
}

#endif //__SPEARMANS_RHO_H
