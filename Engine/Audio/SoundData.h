#pragma once
#include <xaudio2.h>
#pragma comment(lib,"xaudio2.lib")

struct SoundData
{
	WAVEFORMATEX wfex;
	BYTE* pBuffer;
	unsigned int bufferSize;
};