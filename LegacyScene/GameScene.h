#pragma once
#include "Base/EngineCore.h"
#include "IScene.h"
#include "Colliders/CollisionManager.h"

class GameScene : public IScene{
public:
	GameScene(EngineCore* engineCore);
	~GameScene()override;

public:
	void Initialize()override;
	void Update()override;
	void Draw()override;

	IScene* GetNextScene() override;

	void CameraUpdate();

private:
	EngineCore* engineCore_;
	DirectInputManager* input_;
	CollisionManager collisionManager_;
	Camera camera_;

#ifdef _DEBUG
	DebugCamera debugCamera_;
	bool isActiveDebugCamera_;
#endif // _DEBUG
};