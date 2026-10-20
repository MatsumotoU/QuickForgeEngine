#pragma once
#include "SceneObject.h"
#include <cstdint>
#include <memory>
#include <nlohmann/json.hpp>


namespace QFE::SCENE {
	struct SceneSnapshot {
		nlohmann::json sceneJson;
		std::string scenePath;
	};

	/// @brief シーンの管理クラスです.
	class SceneManager final {
	public:
		explicit SceneManager(std::shared_ptr<const QFE::StaticComponentRegistry> registry);
		SceneManager(const SceneManager&) = delete;
		SceneManager& operator=(const SceneManager&) = delete;
		SceneManager(SceneManager&&) = delete;
		SceneManager& operator=(SceneManager&&) = delete;

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
		/// @brief メモリ上のJSONから現在のシーンをロードします.
		bool LoadCurrentSceneFromJson(const nlohmann::json& sceneJson);
		/// @brief 現在のシーンを、ファイルを書き換えずにメモリへ退避します.
		SceneSnapshot CreateSnapshot() const;
		/// @brief DLLアンロード前に、DLL内コードを参照するComponentStorageを破棄します.
		void ReleaseComponentRegistryForReload();
		/// @brief 新しいRegistryへ接続し、退避したシーンを復元します.
		bool RestoreAfterComponentReload(
			std::shared_ptr<const QFE::StaticComponentRegistry> registry,
			const SceneSnapshot& snapshot);
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
		std::shared_ptr<const QFE::StaticComponentRegistry> registry_;
		/// @brief 現在のシーンオブジェクトです.
		SceneObject currentScene_;
		/// @brief 現在のシーンの保存先です. 未保存の場合は空文字列です.
		std::string currentScenePath_;
		uint64_t sceneRevision_ = 0;
	};
}
