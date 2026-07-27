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
}