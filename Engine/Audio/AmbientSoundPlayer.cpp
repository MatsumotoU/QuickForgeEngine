#include "AmbientSoundPlayer.h"

AmbientSoundPlayer::AmbientSoundPlayer() {
	engineCore_ = nullptr;
	ambientSoundSourceVoices_.clear();
}

AmbientSoundPlayer::~AmbientSoundPlayer() {
	for (auto& sourceVoice : ambientSoundSourceVoices_) {
		if (sourceVoice) {
			sourceVoice->DestroyVoice();
		}
	}
	ambientSoundSourceVoices_.clear();
}

void AmbientSoundPlayer::Initialize(EngineCore* engineCore) {
	engineCore_ = engineCore;
}

void AmbientSoundPlayer::LoadAmbientSoundPreset(const std::string& fileName) {
	fileName;
}
