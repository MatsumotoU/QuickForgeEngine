#pragma once


#include <memory>
#include <cstdint>
#include <unordered_map>

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
		/// 対象 ID を省略した場合は従来どおりメインシーンを更新する。
		void Update(SCENE::SceneManager& sceneManager, float deltaTime, uint64_t sceneId = 0);
		float GetFadeAlpha(uint64_t sceneId = 0) const;
		/// シーン全体の退避復元時に、古い遷移先を持ち越さない。
		void Reset();

	private:
		// 別シーンの遷移でフェード状態を共有しない。
		std::unordered_map<uint64_t, std::unique_ptr<ISceneState>> states_;
	};
}
