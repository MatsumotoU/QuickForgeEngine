#pragma once
#include "EngineCore.h"
#include "../../Engine/Math/MyRandomNum.h"
#include "../../Engine/Particle/Particle.h"
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

	Transform spriteTransform_;
	Sprite sprite_;
	uint32_t outsideGH_;
	uint32_t roomGH_;
	uint32_t caveGH_;
	uint32_t waterGH_;

private:
	SoundData waterSoundData_;
	IXAudio2SourceVoice* waterVoice_;
	SoundData windSoundData_;
	IXAudio2SourceVoice* windVoice_;

	nlohmann::json presetData_;
	std::string state_;

	float masterVolume_;
	SoundData soundData_;
	IXAudio2SourceVoice* rightVoice_;
	IXAudio2SourceVoice* leftVoice_;
	float rotateSpeed_;
	float pan_;
	float cutoffOffset_;
	float cutoff_;
	float cutoffRate_;
	float iidRate_;
	float iidVolume_;
	float internalVolumeDifferenceRate_;
	float nowRightVolume_;
	float nowLeftVolume_;
	float playTimeRate_;
	float playTimeDecayRate_;

	float echoVolumeAttenuation_;
	float echoInitialVolumeAttenuation_;
	float echoInitialVolumeDecay_;

	std::vector<IXAudio2SourceVoice*> echoVoice_;
	std::vector<float> echoPlayTime_;

	SoundData mp3Data_;
	IXAudio2SourceVoice* mp3Voice_;

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