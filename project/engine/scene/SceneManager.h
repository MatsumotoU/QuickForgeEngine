#pragma once
#include "SceneObject.h"
#include <cstdint>
#include <nlohmann/json.hpp>


namespace QFE::SCENE {
	/// @brief シーンの管理クラスです.
	class SceneManager final {
	public:
		/// @brief シーンの初期化を行います.
		void Initialize();
		/// @brief 現在のシーンにカメラエンティティを作成します.
		/// 既存のメインカメラがない場合は、作成したカメラをメインカメラにします.
		uint32_t CreateCameraEntity(const std::string& name = "Camera");
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
		uint64_t GetSceneRevision() const { return sceneRevision_; }

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
		uint64_t sceneRevision_ = 0;
	};
}
