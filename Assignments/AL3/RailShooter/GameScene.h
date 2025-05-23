#pragma once
#include "../../../Engine/Base/EngineCore.h"
#include "Class/SkyDome.h"
#include "Class/Player.h"
#include "Class/Bulllet.h"
#include "Class/Enemy.h"
#include "IScene.h"

static inline const uint32_t kPlayerBullets = 32;
static inline const uint32_t kEnemyBullets = 128;
static inline const uint32_t kEnemies = 32;

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
	Bullet playerBullets[kPlayerBullets];
	Enemy enemies[kEnemies];
	Bullet enemyBullets[kEnemyBullets];
};