#pragma once
#include "IScene.h"
#include "Utility/DesignPatterns/Singleton.h"
#include <memory>
#include <string>

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
	bool IsRunningScript() const { return isRunningScript_; }

	void SaveScene(const std::string& sceneName);
	void LoadScene(const std::string& sceneName);
	void ResetScene();

	void AddEpmtyObject();
	void LoadModel(const std::string& modelName);
	void AddSprite(const std::string& spriteName);
	void AddScript(uint32_t entityId,const std::string& scriptName);

	void StartScript();
	void StopScript();

private:
	std::unique_ptr<IScene> currentScene_;
	bool isRunningScript_;
	bool isRequestStopScript_;
};