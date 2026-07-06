#pragma once
#include "SceneObject.h"

namespace QFE::SCENE {
	/// @brief シーンの管理クラスです.
	class SceneManager final {
	public:
		/// @brief シーンの初期化を行います.
		void Initialize();
		/// @brief フレーム終了処理を行います.
		void EndFrame(); 
		/// @brief シーンの終了処理を行います.
		void Shutdown();

		/// @brief 現在のシーンをJSONファイルに保存します.
		void SaveCurrentSceneToJson(const std::string& filePath);
		/// @brief JSONファイルから現在のシーンをロードします.
		void LoadCurrentSceneFromJson(const std::string& filePath);

		/// @brief 現在のシーンのエンティティマネージャーの参照を取得します.
		QFE::EntityManager& GetCurrentSceneEntityManager();

	private:
		/// @brief 現在のシーンオブジェクトです.
		SceneObject currentScene_;
	};
}