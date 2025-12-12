#include "AudioSourceManager.h"
#include "Loader/MultiAudioLoader.h"
#include "AppUtility/FileSystems/FileUtility.h"
#ifdef _DEBUG
#include "AppUtility/DebugTool/DebugLog/MyDebugLog.h"
#endif // _DEBUG

void AudioSourceManager::Initialize() {
	audioDataMap_.clear();
	audioHandleMap_.clear();
	nextHandle_ = 0;
	Multiaudioloader::Initialize();
}

uint32_t AudioSourceManager::LoadSoundData(const std::string& filePath) {
	// すでに読み込まれている場合はハンドルを返す
	auto it = audioHandleMap_.find(filePath);
	if (it != audioHandleMap_.end()) {
		return it->second;
	}

	// ファイルを開く
	AudioData soundData{};
	try{
#ifdef _DEBUG
		DebugLog("LoadSoundData: " + filePath);
#endif // _DEBUG
		soundData = Multiaudioloader::LoadAudioData(filePath);
	}
	catch (const std::exception& e){
#ifdef _DEBUG
		DebugLog("Faild LoadSoundData: " + filePath);
#endif // _DEBUG
		e;
		return 0;
	}
	// ハンドルを生成して保存
	uint32_t handle = nextHandle_++;
	audioDataMap_[handle] = soundData;
	audioHandleMap_[filePath] = handle;
	return handle;
}

AudioData& AudioSourceManager::GetSoundData(uint32_t handle) {
#ifdef _DEBUG
	DebugLog("GetSoundData: " + std::to_string(handle));
#endif // _DEBUG
	if (audioDataMap_.find(handle) == audioDataMap_.end()) {
		assert(false && "Invalid handle");
	}

	return audioDataMap_[handle];
}

void AudioSourceManager::Finalize() {
	Multiaudioloader::Finalize();
	audioDataMap_.clear();
	audioHandleMap_.clear();
}
