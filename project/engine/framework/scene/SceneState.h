#pragma once
#include <memory>
#include <string>
#include <cstdint>

namespace QFE::SCENE {
	class SceneManager;
}

namespace QFE::FRAMEWORK {
	/// @brief シーンの状態を表す。
	class ISceneState {
	public:
		virtual ~ISceneState() = default;
		virtual std::unique_ptr<ISceneState> Update(
			SCENE::SceneManager& sceneManager, uint64_t sceneId, float deltaTime) = 0;
		virtual float GetFadeAlpha() const = 0;
	};

	/// @brief シーンがアイドル状態のときの状態。
	class IdleSceneState final : public ISceneState {
	public:
		std::unique_ptr<ISceneState> Update(
			SCENE::SceneManager& sceneManager, uint64_t sceneId, float deltaTime) override;
		float GetFadeAlpha() const override { return 0.0f; }
	};

	/// @brief シーンがフェードアウト中のときの状態。
	class FadeOutSceneState final : public ISceneState {
	public:
		explicit FadeOutSceneState(std::string targetScenePath);
		std::unique_ptr<ISceneState> Update(
			SCENE::SceneManager& sceneManager, uint64_t sceneId, float deltaTime) override;
		float GetFadeAlpha() const override { return fadeAlpha_; }

	private:
		std::string targetScenePath_;
		float elapsedSeconds_ = 0.0f;
		float fadeAlpha_ = 0.0f;
		static constexpr float fadeDurationSeconds_ = 0.5f;
	};

	/// @brief シーンがフェードイン中のときの状態。
	class FadeInSceneState final : public ISceneState {
	public:
		std::unique_ptr<ISceneState> Update(
			SCENE::SceneManager& sceneManager, uint64_t sceneId, float deltaTime) override;
		float GetFadeAlpha() const override { return fadeAlpha_; }

	private:
		float elapsedSeconds_ = 0.0f;
		float fadeAlpha_ = 1.0f;
		static constexpr float fadeDurationSeconds_ = 0.5f;
	};
}
