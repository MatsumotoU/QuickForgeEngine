#pragma once
#include <string>
#include <unordered_map>
#include "Data/AudioData.h"
#include "Loader/MultiAudioLoader.h"

class AudioSourceManager final {
public:
	void Initialize();
	uint32_t LoadSoundData(const std::string& filePath);
	AudioData& GetSoundData(uint32_t handle);
	void Finalize();

private:
	uint32_t nextHandle_;
	std::unordered_map<std::string, uint32_t> audioHandleMap_;
	std::unordered_map<uint32_t, AudioData> audioDataMap_;
};
