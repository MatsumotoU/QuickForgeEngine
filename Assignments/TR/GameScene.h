#pragma once
#include "../../Engine/Base/EngineCore.h"
#include "../../Engine/Math/MyRandomNum.h"
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
	void EchoVoice();

private:
	float frameCount_;
	EngineCore* engineCore_;
	AudioManager* audio_;

private:
	float masterVolume_;
	SoundData soundData_;
	IXAudio2SourceVoice* rightVoice_;
	IXAudio2SourceVoice* leftVoice_;
	float rotateSpeed_;
	float pan_;
	float cutoff_;
	float cutoffRate_;
	float iidRate_;
	float iidVolume_;
	float internalVolumeDifferenceRate_;
	float nowRightVolume_;
	float nowLeftVolume_;

	float echoVolumeAttenuation_;
	float echoInitialVolumeAttenuation_;

	std::vector<IXAudio2SourceVoice*> echoVoice_;

	uint32_t oldEchoIndex_;
	bool isEcho_;
	float echoTime_;
	float echoInterval_;
	float echoRandMin_;
	float echoRandMax_;
	MyRandomNum echoIntervalRand_;
	bool isMove;

private:
	DebugCamera debugCamera_;
	float matrix[2];
	SoundObj obj_;
	ListenerObj listener_;

	
};