#include "AudioPlayer.h"
#include "Base/EngineCore.h"

#include <xaudio2.h>

void AudioPlayer::Initialize(EngineCore* engineCore) {
	engineCore_ = engineCore;
}

void AudioPlayer::PlayAudio(uint32_t audioHandle, const std::string& soundVoiceFriendryName) {
	IXAudio2SourceVoice* sourceVoice = engineCore_->GetAudioSourceBinder()->GetSourceVoice(soundVoiceFriendryName);
	// 再生時になっていたら止めてから鳴らす
	sourceVoice->Stop();
	
	// 鳴らす
	Audiomanager::SoundPlaySourceVoice(
		engineCore_->GetAudioResourceManager()->GetSoundData(audioHandle),sourceVoice);
}
