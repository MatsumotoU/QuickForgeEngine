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
#ifdef _DEBUG
	debugCamera_.Initialize(engineCore_->GetWinApp(), input_);
#endif // _DEBUG
	isRequestedExit_ = false;
	isActiveDebugCamera_ = false;

	camera_.transform_.translate.z = -20.0f;
	debugCamera_.camera_.transform_.translate.z = -20.0f;
	player_.Initialize(engineCore_);
	for (int i = 0; i < kBullets; i++) {
		bullets[i].Initialize(engineCore_);
	}
}

void GameScene::Update() {
	camera_.Update();
#ifdef _DEBUG
	if (input_->keyboard_.GetTrigger(DIK_P)) {
		isActiveDebugCamera_ = !isActiveDebugCamera_;
	}
	
	if (isActiveDebugCamera_) {
		debugCamera_.Update();
		camera_ = debugCamera_.camera_;
	}
#endif // _DEBUG

	for (int i = 0; i < kBullets; i++) {
		if (bullets[i].GetIsActive()) {
			bullets[i].Update();
		}
	}
	
	player_.Update();

	if (player_.GetIsShot()) {
		for (int i = 0; i < kBullets; i++) {
			if (!bullets[i].GetIsActive()) {
				bullets[i].ShotBullet(player_.transform_.translate, { 0.0f,0.0f,10.0f }, 120);
				break;
			}
		}
		player_.SetIsShot(false);
	}
}

void GameScene::Draw() {
	ImGui::Begin("GameScene");
	ImGui::Text("isDebug: %s", isActiveDebugCamera_ ? "True" : "False");
	ImGui::End();

	skyDome_.Draw(&camera_);
	player_.Draw(&camera_);
	for (int i = 0; i < kBullets; i++) {
		if (bullets[i].GetIsActive()) {
			bullets[i].Draw(&camera_);
		}
	}
	
}

IScene* GameScene::GetNextScene() {
	return new TitleScene(engineCore_);
}
