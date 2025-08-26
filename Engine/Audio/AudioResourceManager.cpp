#include "AudioResourceManager.h"
#include "Base/EngineCore.h"
#include <cassert>

#ifdef _DEBUG
#include "Utility/MyDebugLog.h"
#endif // _DEBUG

AudioResourceManager::~AudioResourceManager() {
	for (SoundData& sd : soundData_) {
		Audiomanager::SoundUnload(&sd);
	}

#ifdef _DEBUG
	debugResourceIndex_ = 0;
#endif // _DEBUG
}

void AudioResourceManager::Initialize(EngineCore* engineCore) {
	engineCore_ = engineCore;
	fileNameLiblary_.Init("AudioResource");
}

bool AudioResourceManager::LoadAudioResource(const std::string& filePath, const std::string& fileName) {

	if (fileNameLiblary_.GetLiblaryIndex(fileName) <= -1) {
		// 読み込んだことのないリソース処理
		std::string path = filePath + fileName;
		soundData_.push_back(Audiomanager::SoundLoad(path.c_str()));
		fileNameLiblary_.AddStringToLiblary(fileName);
#ifdef _DEBUG
		DebugLog(std::format("LoadAudioResouce: {}", filePath + fileName));
#endif // _DEBUG
		return true;
	} else {
		// 読み込んだ事があるリソース処理
#ifdef _DEBUG
		DebugLog(std::format("AlreadyLoaded AudioResouce: {}", filePath + fileName));
#endif // _DEBUG
		return false;
	}
}

bool AudioResourceManager::LoadAudioResource(const std::string& filePath) {
	if (fileNameLiblary_.GetLiblaryIndex(filePath) <= -1) {
		// 読み込んだことのないリソース処理
		soundData_.push_back(Audiomanager::SoundLoad(filePath.c_str()));
		fileNameLiblary_.AddStringToLiblary(filePath);
#ifdef _DEBUG
		DebugLog(std::format("LoadAudioResouce: {}", filePath));
#endif // _DEBUG
		return true;
	} else {
		// 読み込んだ事があるリソース処理
#ifdef _DEBUG
		DebugLog(std::format("AlreadyLoaded AudioResouce: {}", filePath));
#endif // _DEBUG
		return false;
	}
}

uint32_t AudioResourceManager::GetAudioDataHandle(const std::string& fileName) {
	int32_t index = fileNameLiblary_.GetLiblaryIndex(fileName);
	index;
	assert(index >= 0);
	return static_cast<uint32_t>(fileNameLiblary_.GetLiblaryIndex(fileName));
}

SoundData AudioResourceManager::GetSoundData(uint32_t audioDataHandle) {
	assert(audioDataHandle >= 0 && audioDataHandle < soundData_.size());
	return soundData_[audioDataHandle];
}

void AudioResourceManager::AddSoundData(const SoundData& soundData, const std::string& fileName) {
	if (fileNameLiblary_.GetLiblaryIndex(fileName) <= -1) {
		// 読み込んだことのないリソース処理
		soundData_.push_back(soundData);
		fileNameLiblary_.AddStringToLiblary(fileName);
#ifdef _DEBUG
		DebugLog(std::format("LoadAudioResouce: {}", fileName));
#endif // _DEBUG

	} else {
		// 読み込んだ事があるリソース処理
#ifdef _DEBUG
		DebugLog(std::format("AlreadyLoaded AudioResouce: {}", fileName));
#endif // _DEBUG

	}
}

#ifdef _DEBUG
void AudioResourceManager::DrawImGui() {
	ImGui::SliderInt("ResourceIndex", &debugResourceIndex_, 0, static_cast<int>(soundData_.size()) - 1);
	fileNameLiblary_.DrawLiblary();

	DisplayWaveform(
		fileNameLiblary_.GetDatanameFromIndex(debugResourceIndex_),
		soundData_[debugResourceIndex_]);

}
void AudioResourceManager::DisplayWaveform(const std::string& plotName, const SoundData& data) {
	static std::vector<float> samples;
	int sampleCount = data.bufferSize / 2;
	samples.resize(sampleCount);
	for (int i = 0; i < sampleCount; ++i) {
		short value = data.pBuffer[i * 2] | (data.pBuffer[i * 2 + 1] << 8);
		samples[i] = static_cast<float>(value);
	}
	ImGui::PlotLines(plotName.c_str(), samples.data(), sampleCount, 0, nullptr, -32768.0f, 32767.0f, ImVec2(400, 100));
}
#endif // _DEBUG