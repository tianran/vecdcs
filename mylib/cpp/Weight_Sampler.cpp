#include "Weight_Sampler.h"

#include <cmath>

#include "Random_Generator.h"

using namespace std;

Weight_Sampler::Weight_Sampler(): table(new long[0x800001]) {}

Weight_Sampler::~Weight_Sampler() { delete[] table; }

double Weight_Sampler::init(const vector<double> &weights) {
    double sum = 0.0;
    for (vector<double>::size_type i = weights.size(); i --> 0;) {
        sum += weights[i];
    }

    double s = 0.0;
    int lower = 0;
    for (size_t i = 0; i != weights.size(); ++i) {
        s += weights[i];
        int higher = static_cast<int>(floor(s / sum * 0x800000));
        for (; lower <= higher; ++lower) {
            table[lower] = i;
        }
    }
    table[0x800000] = weights.size();

    return sum;
}

long Weight_Sampler::operator()(Random_Generator &random) {
    int i = static_cast<int>(random() & 0x7fffff);
    long w = table[i + 1] - table[i];
    return (1 < w)? table[i] + random(w) : table[i];
}