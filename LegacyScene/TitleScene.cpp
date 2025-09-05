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
	
	mole_ = std::make_unique<Mole>();
	mole_.get()->Initialize(engineCore_,&camera_);
	
	wall_ = std::make_unique<Wall>();
	wall_.get()->Initialize(engineCore_, &camera_);

	titleName_ = std::make_unique<TitleName>();
	titleName_.get()->Initialize(engineCore_, &camera_);

	titleGround_ = std::make_unique<TitleGround>();
	titleGround_.get()->Initialize(engineCore_, &camera_); 

	skyDome_ = std::make_unique<TitleSkyDome>();
	skyDome_.get()->Initialize(engineCore_, &camera_);
}

void TitleScene::Update() {
	DirectInputManager* input = engineCore_->GetInputManager();
	frameCount_++;

	mole_.get()->Update();

	wall_.get()->Update();

	titleName_.get()->Update();

	titleGround_.get()->Update();

	skyDome_.get()->Update();

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

	mole_.get()->Draw();
	wall_.get()->Draw();
	titleName_.get()->Draw();
	titleGround_.get()->Draw();
	skyDome_.get()->Draw();

#ifdef _DEBUG
	debugCamera_.DrawImGui();

	mole_.get()->DebugImGui();
	wall_.get()->DebugImGui();
	titleName_.get()->DebugImGui();
	titleGround_.get()->DebugImGui();
	skyDome_.get()->DebugImGui();
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
