#pragma once

#include <string>

namespace QFE::SCENE {
	class SceneManager;
}

namespace QFE::FRAMEWORK {
	/// @brief SceneChangeComponent のリクエストをフェード付きで処理する。
	class SceneChangeTransitionSystem final {
	public:
		void Update(SCENE::SceneManager& sceneManager, float deltaTime);
		float GetFadeAlpha() const { return fadeAlpha_; }

	private:
		enum class State {
			Idle,
			FadeOut,
			FadeIn
		};

		State state_ = State::Idle;
		std::string targetScenePath_;
		float elapsedSeconds_ = 0.0f;
		float fadeAlpha_ = 0.0f;
		float fadeDurationSeconds_ = 0.5f;
	};
}
