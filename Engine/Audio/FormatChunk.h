#pragma once
#include <xaudio2.h>
#pragma comment(lib,"xaudio2.lib")

#include "ChunkHeader.h"

struct FormatChunk
{
	ChunkHeader chunk;
	WAVEFORMATEX fmt;
};