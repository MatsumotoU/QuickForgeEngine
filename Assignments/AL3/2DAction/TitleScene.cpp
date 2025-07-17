#include "TitleScene.h"
#include "GameScene.h"

TitleScene::TitleScene(EngineCore* engineCore) {
	engineCore_ = engineCore;
}

TitleScene::~TitleScene() {
	engineCore_->GetGraphRenderer()->DeleteCamera();
}

void TitleScene::Initialize() {
	isInitialized_ = true;
	isRequestedExit_ = false;
	model_.Initialize(engineCore_);
	model_.LoadModel("Resources", "Title.obj", COORDINATESYSTEM_HAND_RIGHT);
	camera_.Initialize(engineCore_->GetWinApp());
#ifdef _DEBUG
	debugCamera_.Initialize(engineCore_);
#endif // _DEBUG
	engineCore_->GetGraphRenderer()->SetCamera(&camera_);
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

#ifdef _DEBUG
	debugCamera_.Update();
	camera_ = debugCamera_.camera_;
#endif // _DEBUG
	camera_.Update();
}

void TitleScene::Draw() {
	assert(engineCore_);

	model_.Draw(&camera_);

	engineCore_->GetGraphRenderer()->DrawGrid(50.0f, 50);
}

std::unique_ptr<IScene> TitleScene::GetNextScene() { 
	return std::make_unique<GameScene>(engineCore_);
}
