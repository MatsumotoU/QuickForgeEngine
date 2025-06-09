#include "TitleScene.h"
#include "GameScene.h"

TitleScene::TitleScene(EngineCore* engineCore) {
	engineCore_ = engineCore;
}

TitleScene::~TitleScene() {}

void TitleScene::Initialize() {
	isRequestedExit_ = false;
	model_.Initialize(engineCore_);
	model_.LoadModel("Resources", "Title.obj", COORDINATESYSTEM_HAND_RIGHT);
	camera_.Initialize(engineCore_->GetWinApp());
}

void TitleScene::Update() {
	DirectInputManager* input = engineCore_->GetInputManager();
	frameCount_++;
	titleTransform_.rotate.y += engineCore_->GetDeltaTime();
	titleTransform_.scale.y = 1.0f + sinf(frameCount_ * 0.1f)*0.2f;
	titleTransform_.scale.x = 1.0f + cosf(frameCount_ * 0.1f) * 0.3f;

	if (input->keyboard_.GetTrigger(DIK_SPACE)) {
		isRequestedExit_ = true;
	}

	model_.transform_ = titleTransform_;
	model_.Update();
}

void TitleScene::Draw() {
	model_.Draw( &camera_);
}

IScene* TitleScene::GetNextScene() { return new GameScene(engineCore_); }
