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

	/// @brief ビットがたっているのどこか１つでも一致するかどうか判定する
	bool HasAnyBits(const Bit32& other) const;
	/// @brief ビットがすべて一致するかどうか判定する
	bool HasAllBits(const Bit32& other) const;
	};

	/// @brief 2つのBit32のビットがどこか1つでも一致するかどうか判定する
	bool HasAnyBits(const Bit32& a, const Bit32& b);
	/// @brief 2つのBit32のビットがすべて一致するかどうか判定する
	bool HasAllBits(const Bit32& a, const Bit32& b);
}
