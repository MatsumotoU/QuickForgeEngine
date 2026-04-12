#pragma once
#include <cstdint>

namespace QFE::RANDOM {
	/// @brief XorShift128アルゴリズムを使用した高速な疑似乱数生成器
	class XorShift128 {
	public:
		/// @brief コンストラクタ。シード値を指定して初期化します。
		XorShift128(uint32_t seed = 2463534242U);
		/// @brief 次の乱数を生成します。
		uint32_t Next();

		/// @brief float型の乱数を生成します。0.0f以上1.0f未満の範囲で返します。
		float NextFloat();
		/// @brief int32_t型の乱数を生成します。0以上max未満の範囲で返します。
		int32_t NextInt(int32_t max);
		/// @brief int32_t型の乱数を生成します。min以上max
		int32_t NextInt(int32_t min, int32_t max);
		
	private:
		uint32_t state_[4];
	};
}
