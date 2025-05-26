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
	for (int i = 0; i < kPlayerBullets; i++) {
		playerBullets[i].Initialize(engineCore_);
	}
	for (int i = 0; i < kEnemyBullets; i++) {
		enemyBullets[i].Initialize(engineCore_);
	}

	for (int i = 0; i < kEnemies; i++) {
		enemies[i].Initialize(engineCore_);
	}

	enemies[0].Spawn({ 0.0f,0.0f,30.0f }, { 0.0f,0.0f,-2.0f });
	enemies[1].Spawn({ 1.5f,0.5f,30.0f }, { 0.0f,0.0f,-2.0f });
	enemies[2].Spawn({ -0.5f,-0.5f,30.0f }, { 0.0f,0.0f,-2.0f });

	timeCount_ = 0.0f;
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

	for (int i = 0; i < kPlayerBullets; i++) {
		if (playerBullets[i].GetIsActive()) {
			playerBullets[i].Update();
		}
	}
	for (int i = 0; i < kEnemyBullets; i++) {
		if (enemyBullets[i].GetIsActive()) {
			enemyBullets[i].Update();
		}
	}
	
	player_.Update();

	if (player_.GetIsShot()) {
		for (int i = 0; i < kPlayerBullets; i++) {
			if (!playerBullets[i].GetIsActive()) {
				playerBullets[i].ShotBullet(player_.transform_.translate, Vector3::Transform({ 0.0f,0.0f,10.0f },player_.GetRotateMatrix()) , 120);
				break;
			}
		}
		player_.SetIsShot(false);
	}

	for (int i = 0; i < kEnemies; i++) {
		enemies[i].Update();

		if (enemies[i].GetIsShot()) {
			for (int b = 0; b < kEnemyBullets; b++) {
				if (!enemyBullets[b].GetIsActive()) {
					enemyBullets[b].ShotBullet(enemies[i].transform_.translate, Vector3::Transform({ 0.0f,10.0f,0.0f }, enemies[i].GetRotateMatrix()), 120);
					break;
				}
			}
			enemies[i].SetIsShot(false);
		}
	}

	timeCount_ += engineCore_->GetDeltaTime();
}

void GameScene::Draw() {
	ImGui::Begin("GameScene");
	ImGui::Text("Time %.2f", timeCount_);
	ImGui::Text("isDebug: %s", isActiveDebugCamera_ ? "True" : "False");
	ImGui::Toggle("isActiveDebugCamera", &isActiveDebugCamera_);
	ImGui::End();

	//skyDome_.Draw(&camera_);
	player_.Draw(&camera_);
	for (int i = 0; i < kPlayerBullets; i++) {
		if (playerBullets[i].GetIsActive()) {
			playerBullets[i].Draw(&camera_);
		}
	}
	for (int i = 0; i < kEnemyBullets; i++) {
		if (enemyBullets[i].GetIsActive()) {
			enemyBullets[i].Draw(&camera_);
		}
	}

	for (int i = 0; i < kEnemies; i++) {
		enemies[i].Draw(&camera_);
	}
}

IScene* GameScene::GetNextScene() {
	return new TitleScene(engineCore_);
}
