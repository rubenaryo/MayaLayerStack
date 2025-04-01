#include "randoms.h"

static std::uniform_real_distribution<float> UniformFloatDistrib(0.f, 1.f);

float rand1()
{
    static std::default_random_engine rnd;
    return UniformFloatDistrib(rnd);
}

AtVector2 rand2()
{
    return AtVector2(rand1(), rand1());
}

AtVector rand3()
{
    return AtVector(rand1(), rand1(), rand1());
}