#include "GameScene.h"
#include "TitleScene.h"
#include <functional>

GameScene::GameScene(EngineCore* engineCore) :debugCamera_(engineCore) {
	engineCore_ = engineCore;
	input_ = engineCore_->GetInputManager();

#ifdef _DEBUG
	isActiveDebugCamera_ = false;
	debugCamera_.Initialize(engineCore_);
	debugCamera_.camera_.transform_.translate.z = -20.0f;
#endif // _DEBUG

	engineCore_->GetGraphRenderer()->SetCamera(&camera_);
	engineCore_->GetLoopStopper()->AddNonStoppingFunc(std::bind(&GameScene::CameraUpdate, this));

	sceneID_ = 1;

	map_ = MapChipLoader::Load("Resources/Map/Stage1.csv");
}

GameScene::~GameScene() {
	engineCore_->GetGraphRenderer()->DeleteCamera(&camera_);
}

void GameScene::Initialize() {
	camera_.Initialize(engineCore_->GetWinApp());
	isRequestedExit_ = false;
	collisionManager_.Initalize();

	floorChip_.Initialize(engineCore_, &camera_);
	wallChip_.Initialize(engineCore_, &camera_);
	floorChip_.SetMapPosition({ -3.5f,0.0f,-3.5f });
	wallChip_.SetMapPosition({ -3.5f,1.0f,-3.5f });
}

void GameScene::Update() {
	camera_.Update();
#ifdef _DEBUG
	if (input_->keyboard_.GetTrigger(DIK_P)) {
		isActiveDebugCamera_ = !isActiveDebugCamera_;
	}
	CameraUpdate();
#endif // _DEBUG

	wallChip_.SetMap(map_);

	floorChip_.Update();
	wallChip_.Update();
}

void GameScene::Draw() {
#ifdef _DEBUG
	debugCamera_.DrawImGui();
#endif // _DEBUG

	floorChip_.Draw();
	wallChip_.Draw();
}


IScene* GameScene::GetNextScene() {
	return new TitleScene(engineCore_);
}

void GameScene::CameraUpdate() {
#ifdef _DEBUG
	if (isActiveDebugCamera_) {
		debugCamera_.Update();
		camera_ = debugCamera_.camera_;
	}
#endif // _DEBUG
}
