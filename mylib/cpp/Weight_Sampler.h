#ifndef __WEIGHT_SAMPLER_H
#define __WEIGHT_SAMPLER_H

#include <vector>

class Random_Generator;

class Weight_Sampler {

    long * const table;

public:

    Weight_Sampler();
    ~Weight_Sampler();

    double init(const std::vector<double>& weights);

    long operator()(Random_Generator& random);
};

#endif //__WEIGHT_SAMPLER_H
