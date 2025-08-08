#pragma once
#include <string>
#include <vector>
#include <memory>
#include "IScene.h"

#include "Object/Entity/EntityManager.h"

#include "Camera/Camera.h"
#ifdef _DEBUG
#include "Camera/DebugCamera.h"

#endif // _DEBUG

class EngineCore;
class Collider;

class SceneObject :public IScene {
public:
	SceneObject() = delete;
	SceneObject(EngineCore* enginecore,const std::string& sceneName);
	~SceneObject();

	void Initialize() override;
	void Update() override;
	void Draw() override;

public:
	IScene* GetNextScene() override;
	std::string GetSceneName() const {
		return sceneName_;
	}
	Camera& GetMainCamera() {
		return mainCamera_;
	}
	void SetSceneName(const std::string& sceneName) {
		sceneName_ = sceneName;
	}
	
private:
	EngineCore* engineCore_;
	std::string sceneName_;

	Camera mainCamera_;
#ifdef _DEBUG
	DebugCamera debugCamera_;
#endif // _DEBUG

	EntityManager entityManager_;
};