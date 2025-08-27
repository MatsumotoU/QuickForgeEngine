#include "AudioPlayer.h"
#include "Base/EngineCore.h"

#include <xaudio2.h>

void AudioPlayer::Initialize(EngineCore* engineCore) {
	engineCore_ = engineCore;
}

void AudioPlayer::StopAudio(const std::string& soundVoiceFriendryName) {
	IXAudio2SourceVoice* sourceVoice = engineCore_->GetAudioSourceBinder()->GetSourceVoice(soundVoiceFriendryName);
	sourceVoice->Stop();
	sourceVoice->ExitLoop();
	sourceVoice->FlushSourceBuffers();
}

void AudioPlayer::PlayAudio(uint32_t audioHandle, const std::string& soundVoiceFriendryName, bool isLoop) {
	IXAudio2SourceVoice* sourceVoice = engineCore_->GetAudioSourceBinder()->GetSourceVoice(soundVoiceFriendryName);
	// 再生時になっていたら止めてから鳴らす
	sourceVoice->Stop();
	sourceVoice->ExitLoop();
	sourceVoice->FlushSourceBuffers();
	
	// 鳴らす
	Audiomanager::SoundPlaySourceVoice(
		engineCore_->GetAudioResourceManager()->GetSoundData(audioHandle),sourceVoice, isLoop);
}

void AudioPlayer::PlayAudio(uint32_t audioHandle, const std::string& soundVoiceFriendryName, float loopBeginSecond, float loopSecond, uint32_t loopCount) {
	IXAudio2SourceVoice* sourceVoice = engineCore_->GetAudioSourceBinder()->GetSourceVoice(soundVoiceFriendryName);
	// 再生時になっていたら止めてから鳴らす
	sourceVoice->ExitLoop();
	sourceVoice->Stop();

	// 鳴らす
	Audiomanager::SoundPlayLoopSourceVoice(
		engineCore_->GetAudioResourceManager()->GetSoundData(audioHandle), sourceVoice, loopBeginSecond,loopSecond,loopCount);
}
