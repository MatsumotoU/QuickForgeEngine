/**
 * @file SceneManager.h
 * @brief シーンとエンティティの管理を行うクラス
 */

#pragma once
#include "IScene.h"
#include "engine/include/utility/DesignPatterns/Singleton.h"
#include "engine/include/utility/ID/UniqueIDManager.h"
#include "engine/include/scene/Data/SceneState.h"

#include "engine/include/core/Math/Vector/Vector2.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <nlohmann/json.hpp>

namespace QFE {
	/**
	 * @class SceneManager
	 * @brief シーンの遷移、更新、描画、およびエンティティの生成・管理を行うシングルトンクラス
	 */
	class SceneManager final : public Singleton<SceneManager> {
		friend class Singleton<SceneManager>;

	public:
		/** @brief 初期化処理 */
		void Initialize();
		/** @brief 更新処理 */
		void Update();
		/** @brief 描画前処理 */
		void PreDraw();
		/** @brief 描画処理 */
		void Draw();
		/** @brief 描画後処理 */
		void PostDraw();
		/** @brief フレーム終了時の処理 */
		void EndFrame();
		/** @brief 終了処理 */
		void Finalize();

		/** @brief 現在のシーン名を取得 */
		std::string GetCurrentSceneName() const { return currentScene_->GetSceneName(); }
		/** @brief 名前からエンティティIDを取得 */
		uint32_t GetEntityByName(const std::string& entityName) const;
		/** @brief 一意なIDからエンティティIDを取得 */
		uint32_t GetEntityByUniqueID(uint32_t uniqueId) const;
		/** @brief スクリプトが実行中かどうか */
		bool IsRunningScript() const { return currentScene_->IsRunningScript(); }

		/** @brief エンティティマネージャを取得 */
		EntityManager* GetEntityManager() const { return currentScene_->GetEntityManager(); }
		/** @brief スクリプト実行環境を取得 */
		CsharpScriptExecutor* GetCsharpScriptExecutor() const { return currentScene_->GetCsharpScriptExecutor(); }

		/** @brief シーンの保存 */
		void SaveScene(const std::string& sceneName);
		/** @brief シーンの読み込み */
		void LoadScene(const std::string& sceneName);
		/** @brief シーンのバイナリ化 */
		void SaveSceneBinary(const std::string& sceneName);

		/** @brief プロジェクトごとリセット*/
		void ResetProject(const std::string& projectName);
		/** @brief シーンのリセット */
		void ResetScene();
		/** @brief 実行時のシーン切り替え */
		void RunTimeSwapScene(const std::string& sceneName);

		/** @brief エンティティの削除 */
		void DeleteEntity(uint32_t entityId);
		/** @brief エンティティのコピー */
		void CopyEntity(uint32_t sourceEntityId);
		/** @brief エンティティのモデル変更 */
		void ChangeEntityModel(uint32_t entityId, const std::string& modelName);
		/** @brief エンティティのメッシュ変更 */
		void ChangeEntityMesh(uint32_t entityId, const std::string& meshName);
		/** @brief エンティティの保存 */
		void SaveEntity(uint32_t entityId, const std::string& entityFileName);
		/** @brief 親子関係の設定 */
		void ParentChild(uint32_t parentId, uint32_t childId);
		/** @brief 親子関係の解除 */
		void Unparent(uint32_t childId);

		/** @brief エンティティのシリアライズ */
		void SerializeEntity(uint32_t entityId, nlohmann::json& entityJson);
		/** @brief エンティティのデシリアライズ */
		void DeserializeEntity(uint32_t entityId, const nlohmann::json& entityJson);

		/** @brief 空のオブジェクトを追加 */
		void AddEmptyObject();
		/** @brief パーティクルエミッタを追加 */
		void AddParticleEmitter(const std::string& modelName, uint32_t maxCount);
		/** @brief モデルを追加 */
		void AddModel(const std::string& modelName);
		/** @brief スプライトを追加 */
		void AddSprite(const std::string& spriteName, float width = 0.0f, float height = 0.0f, int inEntityId = -1, int layer = -1, Vector2 pvot = { 0.0f,0.0f });
		/** @brief C#スクリプトを追加 */
		void AddCsharpScript(uint32_t entityId, const std::string& className);
		/** @brief エンティティを追加 */
		uint32_t AddEntity(const std::string& entityName);
		/** @brief 実行時にエンティティを追加 */
		uint32_t RunTimeAddEntity(const std::string& entityName);

		/** @brief シングローバルのシーンデータを取得 */
		nlohmann::json& GetSceneGlobalData() { return sceneGlobalData_; }
		/** @brief スコアを設定 */
		void SetScore(int score) { score_ = score; }
		/** @brief スコアを取得 */
		int GetScore() const { return score_; }

		/** @brief スクリプトの開始 */
		void StartScript();
		/** @brief スクリプトの停止 */
		void StopScript();

		float initTime_;
		float updateTime_;
		float preDrawTime_;
		float drawTime_;
		float postDrawTime_;

	private:
		int score_;
		nlohmann::json sceneConfig_;
		nlohmann::json sceneGlobalData_;
		std::unique_ptr<IScene> currentScene_;
		std::unique_ptr<IScene> nextScene_;

		std::string nextSceneName_;

		SceneState currentSceneState_;
		std::vector<std::function<void()>> sceneState_;
		/// @brief はじめのシーンをロード
		void FirstLoadScene();
		/// @brief シーンの更新
		void RunningScene();
		/// @brief シーンの遷移
		void TransitioningScene();
	};

}
