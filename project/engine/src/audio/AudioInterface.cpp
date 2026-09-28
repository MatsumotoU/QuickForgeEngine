#include "engine/include/audio/AudioInterface.h"
#include "engine/include/assets/AssetManager.h"
#include <cassert>

void AudioInterface::Initialize() {
	pendingSounds_.clear();
	timeUntilNextPlay_.clear();
	burstElapsedSeconds_.clear();
	soundDurationSeconds_.clear();
	hasStartedBurst_.clear();
	minimumFrameSpacingSeconds_ = kMinimumAudioFrameSeconds;
	isPaused_ = false;
	xAudioCore_.Initialize();
	audioChipManager_.Initialize(xAudioCore_.GetXAudio2(), xAudioCore_.GetMasterVoice());
}

void AudioInterface::Finalize() {
	pendingSounds_.clear();
	timeUntilNextPlay_.clear();
	burstElapsedSeconds_.clear();
	soundDurationSeconds_.clear();
	hasStartedBurst_.clear();
	isPaused_ = false;
	audioChipManager_.Finalize();
	xAudioCore_.Finalize();
}

void AudioInterface::Update(float deltaTime) {
	if (isPaused_) {
		return;
	}
	if (deltaTime < 0.0f) {
		deltaTime = 0.0f;
	}
	minimumFrameSpacingSeconds_ = deltaTime;
	if (minimumFrameSpacingSeconds_ < kMinimumAudioFrameSeconds) {
		minimumFrameSpacingSeconds_ = kMinimumAudioFrameSeconds;
	}

	for (auto& cooldown : timeUntilNextPlay_) {
		auto& timeRemaining = cooldown.second;
		timeRemaining -= deltaTime;
		if (timeRemaining < 0.0f) {
			timeRemaining = 0.0f;
		}

		auto elapsedIt = burstElapsedSeconds_.find(cooldown.first);
		const auto startedIt = hasStartedBurst_.find(cooldown.first);
		if (elapsedIt != burstElapsedSeconds_.end() &&
			startedIt != hasStartedBurst_.end() && startedIt->second) {
			elapsedIt->second += deltaTime;
		}
	}

	for (auto pendingIt = pendingSounds_.begin(); pendingIt != pendingSounds_.end();) {
		auto& queue = pendingIt->second;
		const uint32_t audioDataHandle = pendingIt->first;
		float& timeRemaining = timeUntilNextPlay_[audioDataHandle];
		float& elapsed = burstElapsedSeconds_[audioDataHandle];
		bool& hasStarted = hasStartedBurst_[audioDataHandle];
		const auto durationIt = soundDurationSeconds_.find(audioDataHandle);
		const float duration = durationIt != soundDurationSeconds_.end()
			? durationIt->second
			: GetSoundDurationSeconds(audioDataHandle);

		if (hasStarted && elapsed > duration + kAudioScheduleToleranceSeconds) {
			queue.clear();
		}

		if (!queue.empty() && timeRemaining <= 0.0f) {
			const PendingSound request = queue.front();
			queue.pop_front();
			PlaySoundImmediately(request.audioDataHandle, request.loop, request.volume);
			if (!hasStarted) {
				hasStarted = true;
				elapsed = 0.0f;
			}
			timeRemaining = request.spacingAfter;
		}

		if (queue.empty()) {
			pendingIt = pendingSounds_.erase(pendingIt);
		} else {
			++pendingIt;
		}
	}
}

void AudioInterface::StopAllSound() {
	audioChipManager_.StopAllSound();
	pendingSounds_.clear();
	timeUntilNextPlay_.clear();
	burstElapsedSeconds_.clear();
	soundDurationSeconds_.clear();
	hasStartedBurst_.clear();
}

void AudioInterface::PauseAllSound() {
	audioChipManager_.PauseAllSound();
	isPaused_ = true;
}

void AudioInterface::ResumeAllSound() {
	audioChipManager_.ResumeAllSound();
	isPaused_ = false;
}

uint32_t AudioInterface::PlaySoundForAudioData(uint32_t audioDataHandle, bool loop, float volume) {
	const auto cooldownIt = timeUntilNextPlay_.find(audioDataHandle);
	const auto pendingIt = pendingSounds_.find(audioDataHandle);
	const bool hasPendingSound = pendingIt != pendingSounds_.end() && !pendingIt->second.empty();
	if (!isPaused_ && !hasPendingSound &&
		(cooldownIt == timeUntilNextPlay_.end() || cooldownIt->second <= 0.0f)) {
		const uint32_t playHandle = PlaySoundImmediately(audioDataHandle, loop, volume);
		timeUntilNextPlay_[audioDataHandle] = kSameSoundSpacingSeconds;
		burstElapsedSeconds_[audioDataHandle] = 0.0f;
		soundDurationSeconds_[audioDataHandle] = GetSoundDurationSeconds(audioDataHandle);
		hasStartedBurst_[audioDataHandle] = true;
		return playHandle;
	}

	auto& queue = pendingSounds_[audioDataHandle];
	queue.push_back({ audioDataHandle, loop, volume, kSameSoundSpacingSeconds });
	if (soundDurationSeconds_.find(audioDataHandle) == soundDurationSeconds_.end()) {
		soundDurationSeconds_[audioDataHandle] = GetSoundDurationSeconds(audioDataHandle);
		burstElapsedSeconds_[audioDataHandle] = 0.0f;
		hasStartedBurst_[audioDataHandle] = false;
	}

	const float duration = soundDurationSeconds_[audioDataHandle];
	const float elapsed = burstElapsedSeconds_[audioDataHandle];
	const float availableDuration = duration - elapsed;
	if (availableDuration <= 0.0f) {
		queue.clear();
		return 0;
	}

	std::size_t maxQueuedSounds = static_cast<std::size_t>(availableDuration / minimumFrameSpacingSeconds_);
	while (queue.size() > maxQueuedSounds) {
		queue.pop_back();
	}
	if (queue.empty()) {
		return 0;
	}

	float spacing = kSameSoundSpacingSeconds;
	const float idealSpacing = availableDuration / static_cast<float>(queue.size());
	if (idealSpacing < kSameSoundSpacingSeconds) {
		int frameSteps = static_cast<int>(idealSpacing / minimumFrameSpacingSeconds_);
		if (frameSteps < 1) {
			frameSteps = 1;
		}
		spacing = static_cast<float>(frameSteps) * minimumFrameSpacingSeconds_;
	}
	for (auto& request : queue) {
		request.spacingAfter = spacing;
	}

	float& timeRemaining = timeUntilNextPlay_[audioDataHandle];
	if (timeRemaining > spacing) {
		timeRemaining = spacing;
	}
	return 0;
}

uint32_t AudioInterface::PlaySoundImmediately(uint32_t audioDataHandle, bool loop, float volume) {
	const AudioData& audioData = AssetManager::GetInstance()->GetAudioSourceManager()->GetSoundData(audioDataHandle);
	if (audioData.buffer.empty()) { // pBuffer から buffer.empty() に変更
		assert(false && "Audio data is empty"); // アサートメッセージも修正
		return 0;
	}
	return audioChipManager_.PlaySoundForAudioData(audioData, loop, volume);
}

float AudioInterface::GetSoundDurationSeconds(uint32_t audioDataHandle) const {
	const AudioData& audioData = AssetManager::GetInstance()->GetAudioSourceManager()->GetSoundData(audioDataHandle);
	const uint32_t bytesPerSecond = audioData.wfxEx.Format.nAvgBytesPerSec;
	if (bytesPerSecond == 0) {
		return kSameSoundSpacingSeconds;
	}
	return static_cast<float>(audioData.buffer.size()) / static_cast<float>(bytesPerSecond);
}

void AudioInterface::StopSound(uint32_t soundHandle) {
	audioChipManager_.StopSound(soundHandle);
}

void AudioInterface::PauseSound(uint32_t soundHandle) {
	audioChipManager_.PauseSound(soundHandle);
}

void AudioInterface::ResumeSound(uint32_t soundHandle) {
	audioChipManager_.ResumeSound(soundHandle);
}

void AudioInterface::SetMasterVolume(float volume) {
	xAudioCore_.SetMasterVolume(volume);
}

void AudioInterface::SetBGMVolume(float volume) {
	volume;
}

void AudioInterface::SetSEVolume(float volume) {
	volume;
}

void AudioInterface::SetVoiceVolume(float volume) {
	volume;
}

void AudioInterface::SetASVolume(float volume) {
	volume;
}

float AudioInterface::GetMasterVolume() {
	return 0.0f;
}

float AudioInterface::GetBGMVolume() {
	return 0.0f;
}

float AudioInterface::GetSEVolume() {
	return 0.0f;
}

float AudioInterface::GetVoiceVolume() {
	return 0.0f;
}

float AudioInterface::GetASVolume() {
	return 0.0f;
}
