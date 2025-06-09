#pragma once
#include <string>
#include <memory>
#include <vector>
#include <xaudio2.h>

#include "AudioMaterial.h"

class EngineCore;

// TODO: プリセットから環境音を読み込み、再生するクラス

class AmbientSoundPlayer final {
public:
	AmbientSoundPlayer();
	~AmbientSoundPlayer();

public:
	void Initialize(EngineCore* engineCore);
	void LoadAmbientSoundPreset(const std::string& fileName);

private:
	EngineCore* engineCore_;

private:
	std::vector<IXAudio2SourceVoice*> ambientSoundSourceVoices_;

};