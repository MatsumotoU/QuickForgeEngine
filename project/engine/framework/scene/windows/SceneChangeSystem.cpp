#include "framework/scene/SceneChangeSystem.h"
#include "framework/scene/SceneState.h"

#include <utility>

namespace QFE::FRAMEWORK {
	SceneChangeTransitionSystem::SceneChangeTransitionSystem()
		: state_(std::make_unique<IdleSceneState>()) {}

	SceneChangeTransitionSystem::~SceneChangeTransitionSystem() = default;

	float SceneChangeTransitionSystem::GetFadeAlpha() const {
		return state_->GetFadeAlpha();
	}

	void SceneChangeTransitionSystem::Update(SCENE::SceneManager& sceneManager, float deltaTime) {
		// 更新しながら次の遷移を探す
		std::unique_ptr<ISceneState> next = state_->Update(sceneManager, deltaTime);
		if (!next) return;
		// 遷移がある場合は即座に次の状態に移行する
		state_ = std::move(next);
	}
}
