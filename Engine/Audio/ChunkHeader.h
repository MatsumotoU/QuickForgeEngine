#pragma once
#include "stdint.h"
#include <xaudio2.h>
#pragma comment(lib,"xaudio2.lib")

struct ChunkHeader {
	char id[4];
	int32_t size;
};