#pragma once
#include <cstdint>
namespace QFE::MATH {
	/// @brief 32ビットのビットフラグを表す構造体
	class Bit32 {
	public:
	uint32_t value;
	/// @brief すべてのビットを1にする
	void Fill();
	/// @brief すべてのビットを0にする
	void Clear();
	/// @brief 指定したビットをセットする
	void SetBit(int bitIndex);
	/// @brief 指定したビットをクリアする
	void ClearBit(int bitIndex);
	};
}
