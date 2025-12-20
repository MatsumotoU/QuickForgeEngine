#include "engine/include/assets/AudioSource/AudioSourceManager.h"
#include "engine/include/assets/AudioSource/Loader/MultiAudioLoader.h"
#include "engine/include/utility/FileSystems/FileUtility.h"
#ifdef _DEBUG
#include "engine/include/utility/DebugTool/DebugLog/MyDebugLog.h"
#endif // _DEBUG

void AudioSourceManager::Initialize() {
	audioDataMap_.clear();
	audioHandleMap_.clear();
	nextHandle_ = 0;
	Multiaudioloader::Initialize();
}

uint32_t AudioSourceManager::LoadSoundData(const std::string& filePath) {
	// 縺吶〒縺ｫ隱ｭ縺ｿ霎ｼ縺ｾ繧後※縺・ｋ蝣ｴ蜷医・繝上Φ繝峨Ν繧定ｿ斐☆
	auto it = audioHandleMap_.find(filePath);
	if (it != audioHandleMap_.end()) {
		return it->second;
	}

	// 繝輔ぃ繧､繝ｫ繧帝幕縺・
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
	// 繝上Φ繝峨Ν繧堤函謌舌＠縺ｦ菫晏ｭ・
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
