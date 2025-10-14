#pragma once
#include "stdint.h"
#include <xaudio2.h>

// チャンクヘッダを表す構造体
struct ChunkHeader final{
	char id[4];
	int32_t size;
};