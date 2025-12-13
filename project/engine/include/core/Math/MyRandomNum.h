#pragma once
#include <vector>
#include <cmath>
#include <algorithm>

class MyRandomNum final{
public:
	MyRandomNum();

public:
	/// <summary>
	/// 驕主悉莉ｻ諢上・蝗樊焚縺ｧ荳讒伜・蟶・↑荵ｱ謨ｰ繧堤函謌舌＠縺ｾ縺・
	/// </summary>
	/// <param name="min">min</param>
	/// <param name="max">max</param>
	/// <returns></returns>
	float GetUniformDistributionRand(float min,float max);

private:
	std::vector<float> samplers_;
	int maxSamplers_;
};
