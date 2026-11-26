#pragma once
#include "SceneObject.h"
#include <cstdint>
#include <memory>
#include <nlohmann/json.hpp>
#include <vector>


namespace QFE::SCENE {
	using SceneId = uint64_t;
	inline constexpr SceneId MainSceneId = 0;
	inline constexpr SceneId InvalidSceneId = UINT64_MAX;

	/// 別シーンも DLL 更新時に、ID と実行状態を維持して復元する。
	struct SubSceneSnapshot {
		SceneId id;
		std::string name;
		std::string scenePath;
		nlohmann::json sceneJson;
		bool updating;
		bool visible;
	};
	struct SceneSnapshot {
		nlohmann::json sceneJson;
		std::string scenePath;
		bool updating = true;
		bool visible = true;
		std::vector<SubSceneSnapshot> subScenes;
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
		/// 平行光エンティティを作成します。
		uint32_t CreateDirectionalLightEntity(const std::string& name = "DirectionalLight");
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
		/// @brief 退避したシーンと保存先を復元します。
		bool RestoreSnapshot(const SceneSnapshot& snapshot);
		/// @brief DLLアンロード前に、DLL内コードを参照するComponentStorageを破棄します.
		void ReleaseComponentRegistryForReload();
		/// @brief 新しいRegistryへ接続し、退避したシーンを復元します.
		bool RestoreAfterComponentReload(
			std::shared_ptr<const QFE::StaticComponentRegistry> registry,
			const SceneSnapshot& snapshot);
		/// @brief 現在のシーンが読み書きされているファイルパスを取得します.
		const std::string& GetCurrentScenePath() const;
		uint64_t GetSceneRevision() const { return GetSceneRevision(activeSceneId_); }

		/// @brief JSONファイルから現在のシーンをロードし、JSONオブジェクトとして返します.
		nlohmann::json LoadCurrentSceneToJson(const std::string& filePath);

		/// @brief JSONオブジェクトから現在のシーンにエンティティをロードします.
		uint32_t LoadEntityOnCurrentSceneFromJsonObject(const std::string& filePath);
		/// 現在のシーンを維持し、指定位置に別のシーンを追加します。
		uint32_t AppendScene(const std::string& filePath, const QFE::MATH::Vector3& origin);

		/// @brief 現在のシーンのエンティティマネージャーの参照を取得します.
		QFE::EntityManager& GetCurrentSceneEntityManager();

		/// 同じ名前のサブシーンは重複作成せず、既存の ID を返す。
		SceneId LoadSubScene(const std::string& name, const std::string& filePath);
		/// 指定シーンだけを置き換え、ほかのシーンには触れない。
		bool ReplaceScene(SceneId id, const std::string& filePath);
		SceneId FindScene(const std::string& name) const;
		QFE::EntityManager* GetSceneEntityManager(SceneId id);
		/// 存在しない ID には nullptr を返す。参照はシーンの置き換え時に無効になる。
		const std::string* GetScenePath(SceneId id) const;
		uint64_t GetSceneRevision(SceneId id) const;
		bool SetSceneUpdating(SceneId id, bool updating);
		bool SetSceneVisible(SceneId id, bool visible);
		bool IsSceneUpdating(SceneId id) const;
		bool IsSceneVisible(SceneId id) const;
		std::vector<SceneId> GetSceneIds() const;
		/// System が処理する対象を切り替える。処理後はメインに戻す。
		bool SetCurrentScene(SceneId id);
		SceneId GetCurrentSceneId() const { return activeSceneId_; }

	private:
		struct SubScene {
			SceneId id;
			std::string name;
			std::string path;
			std::unique_ptr<SceneObject> object;
			bool updating = true;
			bool visible = true;
			uint64_t revision = 1;
		};
		std::vector<SubScene> subScenes_;
		SceneId nextSceneId_ = 1;
		SceneId activeSceneId_ = MainSceneId;
		bool mainUpdating_ = true;
		bool mainVisible_ = true;
		std::shared_ptr<const QFE::StaticComponentRegistry> registry_;
		/// @brief 現在のシーンオブジェクトです.
		SceneObject currentScene_;
		/// @brief 現在のシーンの保存先です. 未保存の場合は空文字列です.
		std::string currentScenePath_;
		uint64_t sceneRevision_ = 0;
		// 再読み込みや ID 再利用後も描画キャッシュが古い状態を使わないようにする。
		uint64_t revisionCounter_ = 0;
	};
}
