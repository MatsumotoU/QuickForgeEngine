#pragma once
#include "SceneObject.h"
#include <nlohmann/json.hpp>


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
		/// @brief 現在のシーンが読み書きされているファイルパスを取得します.
		const std::string& GetCurrentScenePath() const;

		/// @brief JSONファイルから現在のシーンをロードし、JSONオブジェクトとして返します.
		nlohmann::json LoadCurrentSceneToJson(const std::string& filePath);

		/// @brief JSONオブジェクトから現在のシーンにエンティティをロードします.
		uint32_t LoadEntityOnCurrentSceneFromJsonObject(const std::string& filePath);

		/// @brief 現在のシーンのエンティティマネージャーの参照を取得します.
		QFE::EntityManager& GetCurrentSceneEntityManager();

	private:
		/// @brief 現在のシーンオブジェクトです.
		SceneObject currentScene_;
		/// @brief 現在のシーンの保存先です. 未保存の場合は空文字列です.
		std::string currentScenePath_;
	};
}
