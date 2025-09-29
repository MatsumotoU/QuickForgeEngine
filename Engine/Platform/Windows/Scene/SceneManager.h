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
	void Finalize();

	std::string GetCurrentSceneName() const { return currentScene_->GetSceneName(); }

	void SaveScene(const std::string& sceneName);
	void LoadScene(const std::string& sceneName);
	void ResetScene();

	void LoadModel(const std::string& modelName);
	void AddScript(uint32_t entityId,const std::string& scriptName);

private:
	std::unique_ptr<IScene> currentScene_;
};