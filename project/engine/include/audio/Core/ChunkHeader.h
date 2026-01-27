#pragma once
#include "stdint.h"
#include <xaudio2.h>
namespace QFE {
	// チャンクヘッダを表す構造佁E
	struct ChunkHeader final {
		char id[4];
		int32_t size;
	};
}