#pragma once
#include "GameScene.h"
#include "TitleScene.h"
#include "Class/Fade.h"
#include <memory>

class SceneManager {
public:
	SceneManager(EngineCore* engineCore);
	~SceneManager();

public:
	void Initialize();
	void Update();
	void Draw();

private:
	std::unique_ptr<IScene> currentScene_;
	IScene* scene;
	EngineCore* engineCore_;
	Fade fade_;
};