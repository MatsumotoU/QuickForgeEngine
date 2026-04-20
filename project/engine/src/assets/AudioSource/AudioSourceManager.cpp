#include "engine/include/assets/AudioSource/AudioSourceManager.h"
#include "engine/include/assets/AudioSource/Loader/MultiAudioLoader.h"
#include "engine/include/utility/FileSystems/FileUtility.h"
#ifdef QFE_OPTIMIZE_OFF
#include "engine/include/utility/DebugTool/DebugLog/MyDebugLog.h"
#endif // QFE_OPTIMIZE_OFF

using namespace QFE;

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
	AudioData soundData;
	try{
#ifdef QFE_OPTIMIZE_OFF
		DebugLog("LoadSoundData: " + filePath);
#endif // QFE_OPTIMIZE_OFF
		soundData = Multiaudioloader::LoadAudioData(filePath);
	}
	catch (const std::exception& e){
#ifdef QFE_OPTIMIZE_OFF
		DebugLog("Faild LoadSoundData: " + filePath);
#endif // QFE_OPTIMIZE_OFF
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
#ifdef QFE_OPTIMIZE_OFF
	DebugLog("GetSoundData: " + std::to_string(handle));
#endif // QFE_OPTIMIZE_OFF
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
