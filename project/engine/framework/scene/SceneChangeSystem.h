#pragma once


#include <memory>

namespace QFE::SCENE {
	class SceneManager;
}

namespace QFE::FRAMEWORK {
	/// @brief シーンの状態を表す。前方宣言
	class ISceneState;

	/// @brief SceneChangeComponent のリクエストをフェード付きで処理する。
	class SceneChangeTransitionSystem final {
	public:
		SceneChangeTransitionSystem();
		~SceneChangeTransitionSystem();
		void Update(SCENE::SceneManager& sceneManager, float deltaTime);
		float GetFadeAlpha() const;

	private:
		std::unique_ptr<ISceneState> state_;
	};
}
