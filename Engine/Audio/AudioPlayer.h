#pragma once
#include <string>
class EngineCore;

class AudioPlayer {
public:
	void Initialize(EngineCore* engineCore);

public:
	void PlayAudio(uint32_t audioHandle, const std::string& soundVoiceFriendryName,bool isLoop);
	void PlayAudio(uint32_t audioHandle, const std::string& soundVoiceFriendryName,
		float loopBeginSecond, float loopSecond, uint32_t loopCount);
	void StopAudio(const std::string& soundVoiceFriendryName);

private:
	EngineCore* engineCore_;
};