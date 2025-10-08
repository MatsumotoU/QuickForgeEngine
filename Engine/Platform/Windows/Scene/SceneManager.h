#pragma once
#include "IScene.h"
#include "Utility/DesignPatterns/Singleton.h"
#include <memory>
#include <string>
#include <nlohmann/json.hpp>

class SceneManager final : public Singleton<SceneManager> {
	friend class Singleton<SceneManager>;

public:
	void Initalize();
	void Update();
	void PreDraw();
	void Draw();
	void PostDraw();
	void EndFrame();
	void Finalize();

	std::string GetCurrentSceneName() const { return currentScene_->GetSceneName(); }
	uint32_t GetEntityByName(const std::string& entityName) const;
	bool IsRunningScript() const { return isRunningScript_; }

	void SaveScene(const std::string& sceneName);
	void LoadScene(const std::string& sceneName);
	void ResetScene();

	// ここにいるべきじゃない
	void SaveEntity(uint32_t entityId, const std::string& entityFileName);

	void SerializeEntity(uint32_t entityId, nlohmann::json& entityJson);
	void DeserializeEntity(uint32_t entityId, const nlohmann::json& entityJson);

	void AddEpmtyObject();
	void AddModel(const std::string& modelName);
	void AddSprite(const std::string& spriteName,float width = 0.0f,float height = 0.0f,int inEntityId = -1,int layer = -1);
	void AddScript(uint32_t entityId,const std::string& scriptName);
	uint32_t AddEntity(const std::string& entityName);

	void StartScript();
	void StopScript();

private:
	std::unique_ptr<IScene> currentScene_;
	bool isRunningScript_;
	bool isRequestStopScript_;
};