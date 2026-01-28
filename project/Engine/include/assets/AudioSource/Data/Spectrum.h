#pragma once
#include <vector>
namespace QFE {
	struct Spectrum {
		std::vector<float> frequencies; // 周波数成分の配列
		std::vector<float> magnitudes;  // 各周波数成分の振幅
	};
}