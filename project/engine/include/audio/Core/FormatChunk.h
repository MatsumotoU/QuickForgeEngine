#pragma once
#include <xaudio2.h>
#include "ChunkHeader.h"
namespace QFE {
	// フォーマットチャンクを表す構造佁E
	struct FormatChunk final {
		ChunkHeader chunk;
		WAVEFORMATEX fmt;
	};
}