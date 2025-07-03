#pragma once
#include <cstdint>

enum class RNGAlgorithm {  // add more if you like
    LCG,        // Linear Congruential Generator  (32‑bit)
    XorShift,   // Marsaglia Xorshift32
    Lehmer,     // Lehmer (Park–Miller minimal standard, 31‑bit)
    MiddleSquare//bad but cool algorithm
};

class RNG {
public:
    explicit RNG(RNGAlgorithm algo = RNGAlgorithm::XorShift,
                 uint32_t seed  = 0x12345678u) :
        algo_{algo}, state_{seed ? seed : 0xA5A5A5A5u} {}

        explicit RNG(uint32_t seed) : algo_{RNGAlgorithm::XorShift}, state_{seed ? seed : 0xA5A5A5A5u} {}

    /*------------- Range overloads ----------------*/
    int   Range(int   min, int   max = 1)   { return min + int(next() % (uint32_t(max-min) + 1)); }
    float Range(float min = 0.0f, float max = 1.0f) {
        return min + float(double(next()) / 4294967296.0) * (max - min); // 2^32
    }

private:
    RNGAlgorithm algo_;
    uint64_t  state_;          // 64‑bit to avoid overflow in Lehmer

    /* core: produce a 32‑bit unsigned integer */
    uint32_t next() {
        switch (algo_) {
        case RNGAlgorithm::LCG:      return lcg32();
        case RNGAlgorithm::XorShift: return xorshift32();
        case RNGAlgorithm::Lehmer:   return lehmer31();
        case RNGAlgorithm::MiddleSquare: return middle_square();
        }
        return xorshift32();      // default fallback
    }

    uint32_t middle_square() {
        uint64_t squared = uint64_t(state_) * uint64_t(state_); // promote to 64-bit
        squared = (squared >> 16) & 0xFFFFFFFF;             // take the middle 32 bits
        state_ = uint32_t(squared);
        return state_;
    }

    /*----------- concrete generators ------------*/
    uint32_t lcg32() {                     // 32‑bit LCG  (Numerical Recipies)
        constexpr uint32_t A = 1664525u;
        constexpr uint32_t C = 1013904223u;
        state_ = (A * state_ + C);
        return uint32_t(state_);
    }

    uint32_t xorshift32() {                // Xorshift32
        uint32_t x = uint32_t(state_);
        x ^= x << 13;
        x ^= x >> 17;
        x ^= x << 5;
        state_ = x;
        return x;
    }

    uint32_t lehmer31() {                  // Park–Miller “minimal standard”
        constexpr uint64_t A = 48271u;
        constexpr uint64_t M = 2147483647u;   // 2^31-1
        state_ = (A * state_) % M;
        return uint32_t(state_);
    }
};
