#pragma once
#include <xaudio2.h>
#include "ChunkHeader.h"

struct FormatChunk
{
	ChunkHeader chunk;
	WAVEFORMATEX fmt;
};