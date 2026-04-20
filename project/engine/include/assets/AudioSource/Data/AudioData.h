#pragma once
#include <xaudio2.h>
#include "engine/include/core/Memory/SafeVector.h"

namespace QFE {
	// 音声データを格納する構造体
	struct AudioData final {
		WAVEFORMATEXTENSIBLE wfxEx;
		SafeVector<BYTE> buffer;
	};
}