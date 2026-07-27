#include "random.hlsli"

class RandomGenerator
{
    float32_t3 seed;
    float32_t3 Generate3d()
    {
        seed = rand3dTo3d(seed);
        return seed;
    }
    float32_t Generate1d()
    {
        seed = rand3dTo1d(seed);
        return seed.x;
    }
};