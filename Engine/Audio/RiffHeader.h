#pragma once
#include <xaudio2.h>

#include "ChunkHeader.h"

struct RiffHeader
{
	ChunkHeader chunk;
	char type[4];
};