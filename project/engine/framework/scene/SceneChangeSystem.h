#pragma once

namespace QFE::SCENE {
	class SceneManager;
}

namespace QFE::FRAMEWORK {
	/// @brief SceneChangeComponent のリクエストを処理して、指定されたシーンをロードする。
	void UpdateSceneChangeComponents(SCENE::SceneManager& sceneManager);
}
