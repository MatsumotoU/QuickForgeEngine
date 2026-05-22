#pragma once
#include "IScene.h"
#include "engine/include/core/Thread/PriorityTaskDispatcher.h"
#include "Engine/include/scene/SceneCommand/SceneEntityCommandInvoker.h"
#include "engine/include/utility/ID/UniqueIDManager.h"
#include "engine/include/assets/Script/CsharpScriptExecutor.h"

#include "engine/include/core/Math/Vector/Vector2.h"
#include <memory>
#include <string>
#include <set>
#include <unordered_map>
#include <nlohmann/json.hpp>

namespace QFE {

	class AssetManager;

	class SceneObject final : public IScene {
	public:
		SceneObject();
		~SceneObject() override;
		void Initialize() override;
		void Update() override;
		void PreDraw() override;
		void Draw() override;
		void PostDraw() override;
		void EndFrame() override;
		void Finalize() override;

		// シーンのロード、保孁E
		void LoadScene(const std::string& sceneName) override;
		void SaveScene(const std::string& sceneName) override;
		void SaveSceneBinary(const std::string& sceneName) override;
		void ResetScene() override;

		// シーンの再生、停止
		void RunScene() override;
		void PauseScene() override;
		void ResumeScene() override;
		void StopScene() override;

		// シーンにオブジェクトを追加
		void AddEmptyObject() override;
		void AddParticleEmitter(const std::string& modelName, uint32_t maxCount) override;
		void AddSkybox(const std::string& skyboxName) override;
		void AddModel(const std::string& modelName) override;
		void AddSprite(
			const std::string& spriteName, float width = 0.0f, float height = 0.0f,
			int inEntityId = -1, int layer = -1, Vector2 pivot = { 0.0f,0.0f }) override;
		void AddCsharpScript(uint32_t entityId, const std::string& className) override;
		uint32_t AddEntity(const std::string& entityName, bool useCache = false) override;
		uint32_t RunTimeAddEntity(const std::string& entityName) override;

		// シーンにあるオブジェクトを保存、読み込み、変更
		void DeleteEntity(uint32_t entityId) override;
		void CopyEntity(uint32_t sourceEntityId) override;
		void ChangeEntityModel(uint32_t entityId, const std::string& modelName) override;
		void ChangeEntityMesh(uint32_t entityId, const std::string& meshName) override;
		void SaveEntity(uint32_t entityId, const std::string& entityFileName) override;
		void ParentChild(uint32_t parentId, uint32_t childId) override;
		void Unparent(uint32_t childId) override;
		void SerializeEntity(uint32_t entityId, nlohmann::json& entityJson) override;
		void DeserializeEntity(uint32_t entityId, const nlohmann::json& entityJson) override;

		// シーンにあるオブジェクトの情報を取得
		uint32_t GetEntityByName(const std::string& entityName) const override;
		uint32_t GetEntityByUniqueID(uint32_t uniqueId) const override;
		bool IsRunningScript() const override { return isRunningScript_; }
		bool IsPauseScript() const override { return isPauseScript_; }

		EntityManager* GetEntityManager() override { return &entityManager_; }
		CsharpScriptExecutor* GetCsharpScriptExecutor() override { return &csharpScriptExecutor_; }

	private:
		/// baseNameに（数字）を付与して、シーン内で一意な名前を生成する
		std::string CheckUniqueEntityName(const std::string& baseName) const;

		SceneEntityCommandInvoker frameStartCommandInvoker_;
		SceneEntityCommandInvoker updateCommandInvoker_;
		SceneEntityCommandInvoker preDrawCommandInvoker_;
		SceneEntityCommandInvoker drawCommandInvoker_;
		SceneEntityCommandInvoker postDrawCommandInvoker_;

		AssetManager* assetManager_;

		bool isRunningScript_;
		bool isPauseScript_;
		bool isRequestStopScript_;
		ListUniqueIDManager uniqueIdManager_;

		std::set<uint32_t> usedEntityId_;
		EntityManager entityManager_;

		// シーンに存在するエンティティの情報をキャッシュ（エンティティ名 -> エンティティのJSONデータ）
		std::unordered_map<std::string,std::vector<uint8_t>> loadEntitiesBinary_;
		
		// シーン固有のスクリプト実行環境
		CsharpScriptExecutor csharpScriptExecutor_;
	};

}
