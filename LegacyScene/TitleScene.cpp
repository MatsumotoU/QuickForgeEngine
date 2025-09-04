#include "TitleScene.h"
#include "GameScene.h"

TitleScene::TitleScene(EngineCore* engineCore) :debugCamera_(engineCore) {
	engineCore_ = engineCore;
	engineCore_->GetGraphRenderer()->SetCamera(&camera_);

	sceneID_ = 0;
}

TitleScene::~TitleScene() {
	engineCore_->GetGraphRenderer()->DeleteCamera(&camera_);
}

void TitleScene::Initialize() {
#ifdef _DEBUG
	isActiveDebugCamera_ = false;
	debugCamera_.Initialize(engineCore_);
	debugCamera_.camera_.transform_.translate.z = -20.0f;
#endif // _DEBUG

	isRequestedExit_ = false;
	camera_.Initialize(engineCore_->GetWinApp());
}

void TitleScene::Update() {
	DirectInputManager* input = engineCore_->GetInputManager();
	frameCount_++;

	if (input->keyboard_.GetTrigger(DIK_SPACE)) {
		isRequestedExit_ = true;
	}

#ifdef _DEBUG
	if (input->keyboard_.GetTrigger(DIK_P)) {
		isActiveDebugCamera_ = !isActiveDebugCamera_;
	}
	CameraUpdate();
#endif // _DEBUG
}

void TitleScene::Draw() {
	engineCore_->GetGraphRenderer()->DrawGrid();
#ifdef _DEBUG
	debugCamera_.DrawImGui();
#endif // _DEBUG
}

IScene* TitleScene::GetNextScene() { return new GameScene(engineCore_); }

void TitleScene::CameraUpdate() {
#ifdef _DEBUG
	if (isActiveDebugCamera_) {
		debugCamera_.Update();
		camera_ = debugCamera_.camera_;
	}
#endif // _DEBUG
}
