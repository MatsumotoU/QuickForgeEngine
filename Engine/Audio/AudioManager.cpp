#include "AudioManager.h"
#include <cassert>
#include <fstream>

#include "RiffHeader.h"
#include "FormatChunk.h"
#include "ChunkHeader.h"

AudioManager::~AudioManager() {
	xAudio2_.Reset();
}

void AudioManager::Initialize() {
	IXAudio2MasteringVoice* masterVoice;
	HRESULT hr = XAudio2Create(&xAudio2_, 0, XAUDIO2_DEFAULT_PROCESSOR);
	assert(SUCCEEDED(hr));
	hr = xAudio2_.Get()->CreateMasteringVoice(&masterVoice);
	assert(SUCCEEDED(hr));
}

SoundData Audiomanager::SoundLoadWave(const char* filename) {
	std::ifstream file;
	file.open(filename, std::ios_base::binary);
	assert(file.is_open());

	RiffHeader riff;
	file.read((char*)&riff, sizeof(riff));
	if (strncmp(riff.chunk.id, "RIFF", 4) != 0) {
		assert(0);
	}
	if (strncmp(riff.type, "WAVE", 4) != 0) {
		assert(0);
	}

	FormatChunk format = {};
	file.read((char*)&format, sizeof(ChunkHeader));
	if (strncmp(format.chunk.id, "fmt ", 4) != 0) {
		assert(0 && "Error");
	}
	assert(format.chunk.size <= sizeof(format.fmt));
	file.read((char*)&format.fmt, format.chunk.size);

	ChunkHeader data;
	file.read((char*)&data, sizeof(data));
	if (strncmp(data.id, "JUNK", 4) == 0) {
		file.seekg(data.size, std::ios_base::cur);
		file.read((char*)&data, sizeof(data));
	}

	if (strncmp(data.id, "data", 4) != 0) {
		assert(0);
	}
	char* pBuffer = new char[data.size];
	file.read(pBuffer, data.size);
	file.close();

	SoundData soundData = {};
	soundData.wfex = format.fmt;
	soundData.pBuffer = reinterpret_cast<BYTE*>(pBuffer);
	soundData.bufferSize = data.size;
	return soundData;
}

void Audiomanager::SoundUnload(SoundData* soundData) {
	delete[] soundData->pBuffer;
	soundData->pBuffer = 0;
	soundData->bufferSize = 0;
	soundData->wfex = {};
}

void Audiomanager::SoundPlayWave(IXAudio2* xAudio2, const SoundData& soundData) {
	HRESULT hr;
	IXAudio2SourceVoice* pSourceVoice = nullptr;
	hr = xAudio2->CreateSourceVoice(&pSourceVoice, &soundData.wfex);
	assert(SUCCEEDED(hr));

	XAUDIO2_BUFFER buf{};
	buf.pAudioData = soundData.pBuffer;
	buf.AudioBytes = soundData.bufferSize;
	buf.Flags = XAUDIO2_END_OF_STREAM;

	hr = pSourceVoice->SubmitSourceBuffer(&buf);
	assert(SUCCEEDED(hr));
	hr = pSourceVoice->Start();
	assert(SUCCEEDED(hr));
}
