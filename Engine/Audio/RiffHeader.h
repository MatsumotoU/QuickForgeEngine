#pragma once
#include <xaudio2.h>

#include "ChunkHeader.h"

// RIFFヘッダを表す構造体
struct RiffHeader final {
	ChunkHeader chunk;
	char type[4];
};