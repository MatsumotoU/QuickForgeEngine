#pragma once
#include <string>
#include <vector>
#include <memory>
#include "IScene.h"
#include "Object/BaseGameObject.h"

#include "Camera/Camera.h"
#ifdef _DEBUG
#include "Camera/DebugCamera.h"
#endif // _DEBUG

class EngineCore;

class SceneObject :public IScene {
public:
	SceneObject() = delete;
	SceneObject(EngineCore* enginecore,const std::string& sceneName);
	~SceneObject();

	void Initialize() override;
	void Update() override;
	void Draw() override;

public:
	void AddModel(const std::string& directoryPath, const std::string& filename);

public:
	std::vector<std::unique_ptr<BaseGameObject>>& GetGameObjects() {
		return gameObjects_;
	}
	IScene* GetNextScene() override;
	std::string GetSceneName() const {
		return sceneName_;
	}

private:
	EngineCore* engineCore_;
	std::string sceneName_;
	std::vector<std::unique_ptr<BaseGameObject>> gameObjects_;

	Camera mainCamera_;
#ifdef _DEBUG
	DebugCamera debugCamera_;
#endif // _DEBUG
};