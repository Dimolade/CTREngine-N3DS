#pragma once

#include <cmath>

class Math
{
public:
    static float Cosine(float r) { return std::cos(r); }
    static float Sine(float r) { return std::sin(r); }
    static float Abs (float r) { return std::abs(r); }
};
