#include "Random_Generator.h"

Random_Generator::Random_Generator(uint64_t seed) {

    // first seed
    s[0] = seed;

    // second seed: using SplitMix64
    uint64_t z = seed + UINT64_C(0x9E3779B97F4A7C15);
    z = (z ^ (z >> 30)) * UINT64_C(0xBF58476D1CE4E5B9);
    z = (z ^ (z >> 27)) * UINT64_C(0x94D049BB133111EB);
    s[1] = z ^ (z >> 31);

    // jump
    static const uint64_t JUMP[] = {0x8a5cd789635d2dffULL, 0x121fd2155c472f96ULL};
    uint64_t s0 = 0;
    uint64_t s1 = 0;
    for (int i = 0; i < 2; ++i) {
        for (int b = 0; b < 64; ++b) {
            if (JUMP[i] & 1ULL << b) {
                s0 ^= s[0];
                s1 ^= s[1];
            }
            operator()();
        }
    }
    s[0] = s0;
    s[1] = s1;
}

uint64_t Random_Generator::operator()() {
    uint64_t s1 = s[0];
    const uint64_t s0 = s[1];
    s[0] = s0;
    s1 ^= s1 << 23; // a
    s[1] = s1 ^ s0 ^ (s1 >> 18) ^ (s0 >> 5); // b, c
    return s[1] + s0;
}

long Random_Generator::operator()(long len) {
    return static_cast<long>(operator()() % len);
}
