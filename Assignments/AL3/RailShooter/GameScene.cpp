#include "GameScene.h"
#include "TitleScene.h"

GameScene::GameScene(EngineCore* engineCore) {
	engineCore_ = engineCore;
	input_ = engineCore_->GetInputManager();

	isActiveDebugCamera_ = true;
}

GameScene::~GameScene() {
}

void GameScene::Initialize() {
	skyDome_.Initialize(engineCore_);
	camera_.Initialize(engineCore_->GetWinApp());
	debugCamera_.Initialize(engineCore_->GetWinApp(), engineCore_->GetInputManager());
	isRequestedExit_ = false;
}

void GameScene::Update() {
	debugCamera_.Update();
}

void GameScene::Draw() {
	skyDome_.Draw(&camera_);
}

IScene* GameScene::GetNextScene() {
	return new TitleScene(engineCore_);
}
