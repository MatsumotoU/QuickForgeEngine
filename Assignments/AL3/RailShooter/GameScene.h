#pragma once
#include "Base/EngineCore.h"
#include "Class/SkyDome.h"
#include "Class/Player.h"
#include "Class/Bulllet.h"
#include "Class/Enemy.h"
#include "Class/Reticle.h"
#include "Class/LockOn.h"
#include "IScene.h"

#include "Colliders/CollisionManager.h"

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
	bool isLockOn_;

private:
	EngineCore* engineCore_;
	DirectInputManager* input_;

private:
	Camera fpsCamera_;
	bool isFpsCamera_;
	bool isCartesian_;
	Vector3 position_;

#ifdef _DEBUG
	DebugCamera debugCamera_;
	bool isActiveDebugCamera_;
#endif // _DEBUG
	Camera camera_;
	std::vector<Vector3> lailPoints_;
	float cameraT;
	float cameraMoveSpeed_ = 0.1f;
	bool isMoveLail_;
	std::list<TimeCall*> timedCalls_;
	
private:
	int a;
	float second;
	MyTimer audioPlayTimer_;
	GermanNote note;
	uint32_t audioHandle[3];

	Reticle reticle_;
	SkyDome skyDome_;
	Player player_;
	Bullet playerBullets[kPlayerBullets];
	Enemy enemies[kEnemies];
	Bullet enemyBullets[kEnemyBullets];
	Model groundModel_;
	Transform groundTransform_;
	LockOn lockOn_;
	CollisionManager collisionManager_;
	float timeCount_;
};