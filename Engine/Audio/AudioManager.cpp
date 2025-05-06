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
	HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	assert(SUCCEEDED(hr));

	masterVoice_ = nullptr;
	hr = XAudio2Create(&xAudio2_, 0, XAUDIO2_DEFAULT_PROCESSOR);
	assert(SUCCEEDED(hr));
	// ステレオで制作
	hr = xAudio2_.Get()->CreateMasteringVoice(&masterVoice_);
	assert(SUCCEEDED(hr));
	
#ifdef _DEBUG
	DebugLog(ConvertString(std::format(L"MasterVoice->nChannels:{}", GetOutputChannels())));
#endif // _DEBUG

	multiAudioLoader_.Initialize();
}

void AudioManager::SetMasterVolume(float volume) {
	masterVoice_->SetVolume(volume);
}

uint32_t AudioManager::GetOutputChannels() {
	DWORD channelMask = 0;
	masterVoice_->GetChannelMask(&channelMask);

	// チャンネル数を数える
	uint32_t channelCount = 0;
	for (uint32_t i = 0; i < 32; ++i) {
		if (channelMask & (1 << i)) channelCount++;
	}
	return channelCount;
}

IXAudio2MasteringVoice* AudioManager::GetMasterVoice() {
	return masterVoice_;
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

#ifdef _DEBUG
	DebugLog(ConvertString(std::format(L"LoadSoundData->nChannels:{}", soundData.wfex.nChannels)));
#endif // _DEBUG

	return soundData;
}

SoundData Audiomanager::SoundLoadMp3(const char* filename) {
	SoundData mp3Sound{};
	mp3Sound = Multiaudioloader::LoadSoundData(filename);
	return mp3Sound;
}

void Audiomanager::SoundUnload(SoundData* soundData) {
	delete[] soundData->pBuffer;
	soundData->pBuffer = 0;
	soundData->bufferSize = 0;
	soundData->wfex = {};
}

void Audiomanager::SoundPlayWave(IXAudio2* xAudio2, const SoundData& soundData,float volume, float pitch) {
	HRESULT hr;
	IXAudio2SourceVoice* pSourceVoice = CreateSourceVoice(xAudio2, soundData);

	hr = pSourceVoice->SetVolume(volume);
	assert(SUCCEEDED(hr));

	hr = pSourceVoice->SetFrequencyRatio(pitch);
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

void Audiomanager::SoundPlaySourceVoice(const SoundData& soundData,IXAudio2SourceVoice* pSourceVoice) {
	HRESULT hr{};

	XAUDIO2_BUFFER buf{};
	buf.pAudioData = soundData.pBuffer;
	buf.AudioBytes = soundData.bufferSize;
	buf.Flags = XAUDIO2_END_OF_STREAM;

	hr = pSourceVoice->SubmitSourceBuffer(&buf);
	assert(SUCCEEDED(hr));
	hr = pSourceVoice->Start();
	assert(SUCCEEDED(hr));
}

IXAudio2SourceVoice* Audiomanager::CreateSourceVoice(IXAudio2* xAudio2, const SoundData& soundData) {
	IXAudio2SourceVoice* pSourceVoice = nullptr;
	HRESULT hr = xAudio2->CreateSourceVoice(&pSourceVoice, &soundData.wfex, XAUDIO2_VOICE_USEFILTER);
	assert(SUCCEEDED(hr));
	return pSourceVoice;
}
