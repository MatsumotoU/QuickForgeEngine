#pragma once
#include <xaudio2.h>
#include "ChunkHeader.h"

// フォーマットチャンクを表す構造体
struct FormatChunk final {
	ChunkHeader chunk;
	WAVEFORMATEX fmt;
};