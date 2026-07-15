#pragma once
#include <stdint.h>
namespace QFE::INPUT {
	class InputLogData final {
	public:
		uint32_t startFrame_;
		uint32_t endFrame_;
		uint32_t pressedKeyCode_;
	};
}  // namespace QFE::INPUT