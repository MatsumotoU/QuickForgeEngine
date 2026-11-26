#include "framework/scene/SceneChangeSystem.h"
#include "framework/scene/SceneState.h"
#include "scene/SceneManager.h"

#include <utility>

namespace QFE::FRAMEWORK {
	SceneChangeTransitionSystem::SceneChangeTransitionSystem() = default;

	SceneChangeTransitionSystem::~SceneChangeTransitionSystem() = default;

	void SceneChangeTransitionSystem::Reset() { states_.clear(); }

	float SceneChangeTransitionSystem::GetFadeAlpha(uint64_t sceneId) const {
		const auto it = states_.find(sceneId);
		return it == states_.end() ? 0.0f : it->second->GetFadeAlpha();
	}

	void SceneChangeTransitionSystem::Update(SCENE::SceneManager& sceneManager, float deltaTime, uint64_t sceneId) {
		if (sceneManager.GetSceneEntityManager(sceneId) == nullptr) return;
		auto& state = states_[sceneId];
		if (!state) state = std::make_unique<IdleSceneState>();
		// 更新しながら次の遷移を探す
		std::unique_ptr<ISceneState> next = state->Update(sceneManager, sceneId, deltaTime);
		if (!next) return;
		// 遷移がある場合は即座に次の状態に移行する
		state = std::move(next);
	}
}
