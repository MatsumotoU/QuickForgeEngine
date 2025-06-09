#pragma once
#include <xaudio2.h>

// 音声データを格納する構造体
struct SoundData final {
	WAVEFORMATEX wfex;
	BYTE* pBuffer;
	unsigned int bufferSize;
};