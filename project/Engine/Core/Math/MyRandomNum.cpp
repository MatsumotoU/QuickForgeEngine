#include "MyRandomNum.h"


MyRandomNum::MyRandomNum() {
    maxSamplers_ = 1080;
}

float MyRandomNum::GetUniformDistributionRand(float min, float max) {
    
    int r = rand();
    r = std::clamp(r, static_cast<int>(min * 100.0f), static_cast<int>(max * 100.0f));

    if (samplers_.size() == 0) {
        return static_cast<float>(r) * 0.01f;
    }

    int loopCount = 0;
    while (true)
    {
        loopCount++;
        for (float sample : samplers_) {
            if (static_cast<float>(r) != sample) {
                samplers_.push_back(static_cast<float>(r) * 0.01f);
                return static_cast<float>(r) * 0.01f;
            }
        }

        if (loopCount >= maxSamplers_) {
            samplers_.clear();
            break;
        }

        r = rand();
        r = std::clamp(r, static_cast<int>(min * 100.0f), static_cast<int>(max * 100.0f));
    }
    
    return 0.0f;
}
