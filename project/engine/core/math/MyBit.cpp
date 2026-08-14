#include "MyBit.h"

namespace QFE::MATH {
	void Bit32::Fill() {
		value = 0xFFFFFFFF;
	}

	void Bit32::Clear() {
		value = 0;
	}

	void Bit32::SetBit(int bitIndex) {
		if (bitIndex >= 0 && bitIndex < 32) {
			value |= (1u << bitIndex);
		}
	}

	void Bit32::ClearBit(int bitIndex) {
		if (bitIndex >= 0 && bitIndex < 32) {
			value &= ~(1u << bitIndex);
		}
	}
	bool Bit32::HasAnyBits(const Bit32& other) const {
		return (value & other.value) != 0;
	}
	bool Bit32::HasAllBits(const Bit32& other) const {
		return (value & other.value) == other.value;
	}
	bool HasAnyBits(const Bit32& a, const Bit32& b) {
		return (a.value & b.value) != 0;
	}
	bool HasAllBits(const Bit32& a, const Bit32& b) {
		return (a.value & b.value) == b.value;
	}
}