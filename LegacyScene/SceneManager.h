#pragma once
#include "GameScene.h"
#include "TitleScene.h"
#include "StageSelectScene.h"
#include "Class/Fade.h"
#include "Utility/SimpleJson.h"

class SceneManager {
public:
	SceneManager(EngineCore* engineCore);
	~SceneManager();

public:
	void Initialize();
	void Update();
	void Draw();

private:
	IScene* scene;
	EngineCore* engineCore_;
	Fade fade_;

private:
	float value_;
	nlohmann::json json_;
};