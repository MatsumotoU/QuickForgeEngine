#pragma once
#include "../../../Engine/Base/EngineCore.h"
#include "Class/SkyDome.h"
#include "Class/MapChip.h"
#include "Class/Player.h"
#include "Class/CameraContoroller.h"
#include "Class/Enemy.h"
#include "Class/DeathParticle.h"
#include "IScene.h"

class GameScene : public IScene{
public:
	GameScene(EngineCore* engineCore);
	~GameScene()override;

public:
	void Initialize()override;
	void Update()override;
	void Draw()override;

	std::unique_ptr<IScene> GetNextScene() override;

private:
	EngineCore* engineCore_;
	DirectInputManager* input_;

private:
#ifdef _DEBUG
	DebugCamera debugCamera_;
#endif // _DEBUG
	Camera camera;
	
private:
	
	/*SkyDome skyDome_;
	MapChip map_;
	Player player;
	CameraContoroller cameraContoroller_;
	Enemy enemy_;
	DeathParticle deathParticle_;*/
};