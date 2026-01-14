#pragma once
#include <stdint.h>

class InputLogData final {
public:
	uint32_t startFrame_;
	uint32_t endFrame_;
	uint32_t pressedKeyCode_;
};