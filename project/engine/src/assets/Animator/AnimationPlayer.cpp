#include "engine/include/assets/Animator/AnimationPlayer.h"

void QFE::AnimationPlayer::Initialize()
{
	activeAnimations_.clear();
}

void QFE::AnimationPlayer::Finalize()
{
	activeAnimations_.clear();
}

void QFE::AnimationPlayer::Update(float deltaTime)
{
	// 現在再生中のアニメーションを更新
	activeAnimations_.Each([deltaTime](uint32_t id, AnimationPlayClip& playClip) {
		if (playClip.isPlaying) {
			// アニメーションの再生時間を更新
			playClip.currentTime += deltaTime * playClip.playSpeed;
			// アニメーションクリップの総時間を取得
			float totalDuration = playClip.animClip->GetTotalDuration();
			// ループする場合は再生時間を総時間で割った余りにする
			if (playClip.isLoop) {
				playClip.currentTime = fmod(playClip.currentTime, totalDuration);
			} else if (playClip.currentTime >= totalDuration) {
				// ループしない場合は再生時間が総時間を超えたら停止する
				playClip.isPlaying = false;
				QFE_LOG("AnimationPlayer: Animation '" + playClip.animClip->GetName() + "' has finished playing and will be stopped.");
			}
		}
		});
}

void QFE::AnimationPlayer::FrameEnd()
{
	// 再生が終了したアニメーションを破棄するためのIDを収集する
	std::vector<uint32_t> deleteAnimations;
	activeAnimations_.Each([&deleteAnimations](uint32_t id, const AnimationPlayClip& playClip) {
		if (!playClip.isPlaying) {
			deleteAnimations.push_back(id);
			QFE_LOG("AnimationPlayer: Queuing animation '" + playClip.animClip->GetName() + "' for deletion at end of frame.");
		}
		});

	// アニメーションを破棄する
	for(uint32_t& id : deleteAnimations) {
		QFE_LOG("AnimationPlayer: Stopping animation with ID " + std::to_string(id) + " at end of frame.");
		activeAnimations_.erase(id);
	}
}

uint32_t QFE::AnimationPlayer::PlayAnimation(AnimClip* animClip, float playSpeed, float startTime)
{
	// アニメーションクリップがnullptrの場合はエラーを報告して終了
	if(animClip == nullptr) {
		QFE_REPORT_SYSTEM_ERROR("AnimationPlayer: Attempted to play a null animation clip. Aborting.", SystemError::Abort);
		return UINT32_MAX;
	}

	// アニメーションクリップの情報を設定
	AnimationPlayClip playClip;
	playClip.animClip = animClip;
	playClip.playSpeed = playSpeed;
	playClip.startTime = startTime;
	playClip.currentTime = startTime;

	playClip.isLoop = animClip->IsLoop();
	playClip.isPlaying = true;

	QFE_LOG("AnimationPlayer: Playing animation '" + animClip->GetName() + "' at speed " + std::to_string(playSpeed) + " starting at time " + std::to_string(startTime) + ". Looping: " + (playClip.isLoop ? "Yes" : "No"));
    return activeAnimations_.push_back(playClip);
}

void QFE::AnimationPlayer::StopAnimation(uint32_t animationId)
{
	// 指定されたアニメーションIDが存在しない場合はエラーを報告して終了
	if (!activeAnimations_.Contains(animationId)) {
		QFE_LOG("AnimationPlayer: Attempted to stop an animation with invalid ID " + std::to_string(animationId) + ". No such animation is currently playing.");
		return;
	}

	// アニメーションを停止
	activeAnimations_.Remove(animationId);
}

bool QFE::AnimationPlayer::HasAnimation(uint32_t animationId) const
{
	return activeAnimations_.Contains(animationId);
}

QFE::AnimationPlayClip* QFE::AnimationPlayer::GetAnimationPlayClip(uint32_t animationId)
{
	if (!activeAnimations_.Contains(animationId)) {
		QFE_LOG("AnimationPlayer: Attempted to get an animation with invalid ID " + std::to_string(animationId) + ". No such animation is currently playing.");
		return nullptr;
	}

	return activeAnimations_.Get(animationId);
}
