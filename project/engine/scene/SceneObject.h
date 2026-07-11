#pragma once
#include "design-patterns/EntityManager.h"
#include <nlohmann/json.hpp>

namespace QFE::SCENE {
	/// @brief シーンのEntityを保持、保存、ロードするクラスです.
	class SceneObject final {
	public:
		/// @brief シーンの初期化を行います.
		void Initialize();
		/// @brief フレーム終了処理を行います.
		void EndFrame();

		/// @brief JSONファイルにシーンを保存します.
		void SaveSceneToJson(const std::string& filePath);
		/// @brief JSONファイルからシーンをロードします.
		void LoadSceneFromJson(const std::string& filePath);

		/// @brief エンティティマネージャーの参照を取得します.
		QFE::EntityManager& GetEntityManager();

	private:
		/// @brief EntityManagerのインスタンスです.
		QFE::EntityManager entityManager_;
	};
}