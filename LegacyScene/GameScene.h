#pragma once
#include "Base/EngineCore.h"
#include "IScene.h"
#include "Colliders/CollisionManager.h"

#include "Class/GameScene/Map/MapChip.h"
#include "Class/GameScene/Map/MapChipLoader.h"

#include "Class/GameScene/Player.h"

#include "Math/Vector/IntVector2.h"

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
	void MapChipUpdate(GamePlayer& gamePlayer);
	void BuildingMapChipUpdate(GamePlayer& gamePlayer);
	void JumpingUpdate(GamePlayer& gamePlayer);
	void GroundingUpdate(GamePlayer& gamePlayer);

private:
	EngineCore* engineCore_;
	DirectInputManager* input_;
	CollisionManager collisionManager_;
	Camera camera_;

	MapChip floorChip_;
	MapChip wallChip_;
	Player player_;

	std::vector<std::vector<uint32_t>> map_;
	std::vector<IntVector2> buildMapChipIndex_;

#ifdef _DEBUG
	DebugCamera debugCamera_;
	bool isActiveDebugCamera_;
#endif // _DEBUG
};