#pragma once
#include <vector>
#include <cmath>
#include <algorithm>

class MyRandomNum final{
public:
	MyRandomNum();

public:
	/// <summary>
	/// 過去の任意の回数を基に一様分布の乱数を生成します
	/// </summary>
	/// <param name="min">min</param>
	/// <param name="max">max</param>
	/// <returns></returns>
	float GetUniformDistributionRand(float min,float max);

private:
	std::vector<float> samplers_;
	int maxSamplers_;
};
