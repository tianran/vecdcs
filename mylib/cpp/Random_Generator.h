#ifndef __RANDOM_GENERATOR_H
#define __RANDOM_GENERATOR_H

#include <cstdint>

/* xorshift128+ generator initialized by SplitMix64 */
class Random_Generator {

    uint64_t s[2];

public:

    typedef uint64_t result_type;

    static constexpr uint64_t min() { return 1ULL; }
    static constexpr uint64_t max() { return 0xFFFFFFFFFFFFFFFFULL; }

    Random_Generator(uint64_t seed);
    uint64_t operator()();

    long operator()(long len);
};


#endif //__RANDOM_GENERATOR_H
