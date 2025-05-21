#pragma once
#include "../../Engine/Base/EngineCore.h"
#include "SoundObj.h"
#include "ListenerObj.h"

class GameScene {
public:
	GameScene(EngineCore* engineCore);

public:
	void Initialize();
	void Update();
	void Draw();

private:
	float frameCount_;
	EngineCore* engineCore_;
	AudioManager* audio_;

private:
	float masterVolume_;
	SoundData soundData_;
	IXAudio2SourceVoice* sourceVoice_;
	float pan_;

private:
	DebugCamera debugCamera_;
	float matrix[2];
	SoundObj obj_;
	ListenerObj listener_;
};