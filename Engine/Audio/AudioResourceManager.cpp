#include "AudioResourceManager.h"
#include "Base/EngineCore.h"
#include <cassert>

#ifdef _DEBUG
#include "Base/MyDebugLog.h"
#endif // _DEBUG

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

uint32_t AudioResourceManager::GetAudioDataHandle(const std::string& fileName) {
	int32_t index = fileNameLiblary_.GetLiblaryIndex(fileName);

	assert(index >= 0);
	return static_cast<uint32_t>(fileNameLiblary_.GetLiblaryIndex(fileName));
}

SoundData AudioResourceManager::GetSoundData(uint32_t audioDataHandle) {
	assert(audioDataHandle >= 0 && audioDataHandle < soundData_.size());
	return soundData_[audioDataHandle];
}
