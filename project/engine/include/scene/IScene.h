#pragma once
#include <string>
#include <nlohmann/json.hpp>
#include "engine/include/core/Math/Vector/Vector2.h"

class IScene {
public:
	virtual ~IScene() = default;

	// シーンの初期化、更新、描画
	virtual void Initialize() = 0;
	virtual void Update() = 0;
	virtual void PreDraw() = 0;
	virtual void Draw() = 0;
	virtual void PostDraw() = 0;
	virtual void EndFrame() = 0;
	virtual void Finalize() = 0;

	// シーンのロード、保存
	virtual void LoadScene(const std::string& sceneName) = 0;
	virtual void SaveScene(const std::string& sceneName) = 0;
	virtual void ResetScene() = 0;

	// シーンの再生、停止
	virtual void RunScene() = 0;
	virtual void PauseScene() = 0;
	virtual void ResumeScene() = 0;
	virtual void StopScene() = 0;

	// シーンにオブジェクトを追加
	virtual void AddEmptyObject() = 0;
	virtual void AddParticleEmitter(const std::string& modelName,uint32_t maxCount) = 0;
	virtual void AddModel(const std::string& modelName) = 0;
	virtual void AddSprite(const std::string& spriteName, float width = 0.0f, float height = 0.0f, int inEntityId = -1, int layer = -1, Vector2 pivot = { 0.0f,0.0f }) = 0;
	virtual void AddLuaScript(uint32_t entityId, const std::string& scriptName) = 0;
	virtual void AddCsharpScript(uint32_t entityId, const std::string& className) = 0;
	virtual uint32_t AddEntity(const std::string& entityName) = 0;
	virtual uint32_t RunTimeAddEntity(const std::string& entityName) = 0;

	// シーンにあるオブジェクトを保存、読み込み、変更
	virtual void DeleteEntity(uint32_t entityId) = 0;
	virtual void CopyEntity(uint32_t sourceEntityId) = 0;
	virtual void ChangeEntityModel(uint32_t entityId, const std::string& modelName) = 0;
	virtual void ChangeEntityMesh(uint32_t entityId, const std::string& meshName) = 0;
	virtual void SaveEntity(uint32_t entityId, const std::string& entityFileName) = 0;
	virtual void ParentChild(uint32_t parentId, uint32_t childId) = 0;
	virtual void Unparent(uint32_t childId) = 0;
	virtual void SerializeEntity(uint32_t entityId, nlohmann::json& entityJson) = 0;
	virtual void DeserializeEntity(uint32_t entityId, const nlohmann::json& entityJson) = 0;

	// シーンにあるオブジェクトの情報を取得
	virtual uint32_t GetEntityByName(const std::string& entityName) const = 0;
	virtual uint32_t GetEntityByUniqeID(uint32_t uniqueId) const = 0;

	virtual bool IsRunningScript() const = 0;
	std::string& GetSceneName() { return sceneName_; }
	void SetSceneName(const std::string& name) { sceneName_ = name; }
	bool GetReqesytedExit() { return isRequestedExit_; }

protected:
	std::string sceneName_;
	bool isRequestedExit_;
};
