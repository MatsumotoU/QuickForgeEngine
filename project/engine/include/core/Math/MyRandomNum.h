#pragma once
#include <vector>
#include <cmath>
#include <algorithm>

class MyRandomNum final{
public:
	MyRandomNum();

public:
	/// <summary>
	/// 過去任意�E回数で一様�E币E��乱数を生成しまぁE
	/// </summary>
	/// <param name="min">min</param>
	/// <param name="max">max</param>
	/// <returns></returns>
	float GetUniformDistributionRand(float min,float max);

private:
	std::vector<float> samplers_;
	int maxSamplers_;
};
