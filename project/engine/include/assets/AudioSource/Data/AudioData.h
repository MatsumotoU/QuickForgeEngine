#pragma once
#include <xaudio2.h>

// 音声チE�Eタを格納する構造佁E
struct AudioData final {
	WAVEFORMATEX wfex;
	BYTE* pBuffer;
	unsigned int bufferSize;
};
