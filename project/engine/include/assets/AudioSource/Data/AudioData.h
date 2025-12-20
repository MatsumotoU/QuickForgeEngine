#pragma once
#include <xaudio2.h>
#include <vector>

// 音声データを格納する構造体
struct AudioData final {
	WAVEFORMATEXTENSIBLE wfxEx;
	std::vector<BYTE> buffer;
};