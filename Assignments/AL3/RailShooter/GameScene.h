#pragma once
#include "../../../Engine/Base/EngineCore.h"
#include "Class/SkyDome.h"
#include "Class/Player.h"
#include "Class/Bulllet.h"
#include "IScene.h"

class GameScene : public IScene{
public:
	GameScene(EngineCore* engineCore);
	~GameScene()override;

public:
	void Initialize()override;
	void Update()override;
	void Draw()override;

	IScene* GetNextScene() override;

private:
	EngineCore* engineCore_;
	DirectInputManager* input_;

private:
#ifdef _DEBUG
	DebugCamera debugCamera_;
	bool isActiveDebugCamera_;
#endif // _DEBUG
	Camera camera_;
	
private:
	SkyDome skyDome_;
	Player player_;
	Bullet bullet_;
};