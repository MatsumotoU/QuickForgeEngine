#pragma once
#include "Base/EngineCore.h"
#include "IScene.h"
#include "Colliders/CollisionManager.h"

#include "Class/GameScene/Map/MapChip.h"
#include "Class/GameScene/Map/MapChipLoader.h"
#include "Class/GameScene/Map/PredictionLine.h"
#include "Class/GameScene/Map/LandingPoint.h"

#include "Class/GameScene/Player.h"
#include "Class/GameScene/Enemy.h"

#include "Class/GameScene/MainMenu.h"
#include "Class/GameScene/ResultUI.h"
#include "Class/GameScene/TurnText.h"

#include "Class/TitleScene/TitleSkyDome.h"
#include "Class/GameScene/ParticleManager.h"

#include "Class/GameScene/AiLeveler.h"

#include "Math/Vector/IntVector2.h"

#include "Class/GameScene/Number.h"

class GameScene : public IScene{
public:
	GameScene(EngineCore* engineCore, nlohmann::json* data);
	~GameScene()override;

public:
	void Initialize()override;
	void Update()override;
	void Draw()override;

	IScene* GetNextScene() override;

private:
	void ChangeNextScene(IScene* nextScene);

	void MainGameUpdate();
	void MenuUpdate();

	void ResetGame(const std::string& stageName);
	void CameraUpdate();
	void PredictionLineUpdate(GamePlayer& gamePlayer);
	void MapChipUpdate(GamePlayer& gamePlayer);
	void BuildingMapChipUpdate(GamePlayer& gamePlayer);
	void JumpingUpdate(GamePlayer& gamePlayer);
	void GroundingUpdate(GamePlayer& gamePlayer);
	void AliveCheck(GamePlayer& gamePlayer);
	void CheckEndGame();

	uint32_t frameCount_;
	EngineCore* engineCore_;
	DirectInputManager* input_;
	CollisionManager collisionManager_;
	Camera camera_;
	float cameraShakeTimer_;

	bool isOpenMenu_;
	MainMenu mainMenu_;
	ResultUI resultUI_;
	TurnText turnText_;

	MapChip floorChip_;
	MapChip wallChip_;
	PredictionLine predictionLine_;
	LandingPoint landingPoint_;

	bool isPlayerTurn_;
	Player player_;
	Enemy enemy_;
	AiLeveler aiLeveler_;
	bool isEndGame_;
	float endGameTimer_;

	float timer_;

	std::vector<std::vector<uint32_t>> wallMap_;
	std::vector<std::vector<uint32_t>> floorMap_;
	std::vector<IntVector2> buildMapChipIndex_;
	std::vector<IntVector2> oldBuildMapChipIndex_;

	std::string stageName_;
	IScene* nextScene_;

	TitleSkyDome skyDome_;

	uint32_t bgmHandle_;
	ParticleManager particleManager_;

	Number stageNumber_;
	std::unique_ptr<Model> stageTextModel_;
};