/**
 * @file AnimationManager.cpp
 * @brief アニメーション管理クラスの実装(現状はスタブ)
 */

#include "engine/include/assets/Animator/AnimationManager.h"

using namespace QFE;

/** @brief 初期化 */
void AnimationManager::Initialize() {
	nextAnimationId_ = 0;
	animationMap_.clear();
}

/** @brief 更新 */
void AnimationManager::Update() {
}

/** @brief 終了処理 */
void AnimationManager::Finalize() {
}

uint32_t AnimationManager::CreateEmptyAnimationClip(const std::string& name) {
	animationMap_.emplace(nextAnimationId_, AnimClip(name));
	return nextAnimationId_++;
}