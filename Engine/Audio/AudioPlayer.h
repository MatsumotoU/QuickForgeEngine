#pragma once
#include <string>
class EngineCore;

class AudioPlayer {
public:
	void Initialize(EngineCore* engineCore);

public:
	void PlayAudio(uint32_t audioHandle, const std::string& soundVoiceFriendryName);

private:
	EngineCore* engineCore_;
};