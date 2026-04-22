#pragma once
#include "engine/include/core/Memory/SafeVector.h"

namespace QFE {
	struct Spectrum {
		SafeVector<float> frequencies; // 周波数成分の配列
		SafeVector<float> magnitudes;  // 各周波数成分の振幅

		// コンストラクタ
		explicit Spectrum(size_t size = 0) {
			frequencies = SafeVector<float>(size);
			magnitudes = SafeVector<float>(size);
		}
	};
}