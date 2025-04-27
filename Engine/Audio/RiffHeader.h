#pragma once
#include <xaudio2.h>
#pragma comment(lib,"xaudio2.lib")

#include "ChunkHeader.h"

struct RiffHeader
{
	ChunkHeader chunk;
	char type[4];
};