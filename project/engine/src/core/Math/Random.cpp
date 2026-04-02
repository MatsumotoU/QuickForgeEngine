#include "engine/include/core/Math/Random.h"

QFE::RANDOM::XorShift128::XorShift128(uint32_t seed)
{
	state_[0] = seed;
	state_[1] = seed * 1812433253U + 1;
	state_[2] = seed * 1812433253U * 1812433253U + 2;
	state_[3] = seed * 1812433253U * 1812433253U * 1812433253U + 3;
}

uint32_t QFE::RANDOM::XorShift128::Next()
{
    uint32_t tmp = (state_[0] ^ (state_[0] << 15));
	state_[0] = state_[1];
	state_[1] = state_[2];
	state_[2] = state_[3];
	return state_[3] = (state_[3] ^ (state_[3] >> 21)) ^ (tmp ^ (tmp >> 4));
}

float QFE::RANDOM::XorShift128::NextFloat()
{
	// 生成された乱数を0.0f以上1.0f未満の範囲に正規化
	uint32_t randomInt = Next();
	return static_cast<float>(randomInt) / static_cast<float>(UINT32_MAX);
}

int32_t QFE::RANDOM::XorShift128::NextInt(int32_t max)
{
	if (max <= 0) {
		return 0; // maxが0以下の場合は常に0を返す
	}
	return static_cast<int32_t>(Next() % max);
}

int32_t QFE::RANDOM::XorShift128::NextInt(int32_t min, int32_t max)
{
	// minとmaxの関係を確認し、必要に応じて入れ替える
	if (min > max) {
		int32_t temp = min;
		min = max;
		max = temp;
	}
	// maxがminと同じ場合は常にminを返す
	if (max == min) {
		return min;
	}

	// 生成された乱数をmin以上max未満の範囲に正規化
	int32_t range = max - min;
	return min + static_cast<int32_t>(Next() % range);
}
