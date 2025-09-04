#pragma once
#include "Base/EngineCore.h"
#include "IScene.h"
#include "Colliders/CollisionManager.h"

#include "Class/GameScene/Map/MapChip.h"
#include "Class/GameScene/Map/MapChipLoader.h"

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

	MapChip floorChip_;
	MapChip wallChip_;

	std::vector<std::vector<uint32_t>> map_;

#ifdef _DEBUG
	DebugCamera debugCamera_;
	bool isActiveDebugCamera_;
#endif // _DEBUG
};