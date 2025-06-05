#pragma once
#include "stdint.h"
#include <xaudio2.h>

struct ChunkHeader {
	char id[4];
	int32_t size;
};