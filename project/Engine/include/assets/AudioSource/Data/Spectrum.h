#pragma once
#include <vector>
struct Spectrum {
	std::vector<float> frequencies; // 周波数成分の配列
	std::vector<float> magnitudes;  // 各周波数成分の振幅
};