/**
 * @file MyRandomNum.cpp
 * @brief 乱数生成機能の実装
 */

#include "engine/include/core/Math/MyRandomNum.h"


MyRandomNum::MyRandomNum() {
    maxSamplers_ = 1080;
}

/**
 * @brief 指定範囲内の一様分布乱数を取得
 * @param min 最小値
 * @param max 最大値
 * @return 生成された乱数
 * TODO: 非常にメモリ・CPU効率が悪い。std::vectorに対する線形検索とrand()の無限ループの可能性がある。
 *       通常の一様分布であれば std::mt19937 と std::uniform_real_distribution を使用することを推奨。
 */
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
