#include "GameScene.h"
#include "TitleScene.h"

GameScene::GameScene(EngineCore* engineCore) {
	engineCore_ = engineCore;
	input_ = engineCore_->GetInputManager();
	isActiveDebugCamera_ = false;

#ifdef _DEBUG
	debugCamera_.Initialize(engineCore_->GetWinApp(), input_);
	debugCamera_.camera_.transform_.translate.z = -20.0f;
#endif // _DEBUG
}

GameScene::~GameScene() {
}

void GameScene::Initialize() {
	skyDome_.Initialize(engineCore_);
	camera_.Initialize(engineCore_->GetWinApp());

	isRequestedExit_ = false;
	camera_.transform_.translate.y = 8.5f;
	camera_.transform_.translate.z = -20.0f;
	
	player_.Initialize(engineCore_);
	player_.SetMask(0x00000001);

	for (int i = 0; i < kPlayerBullets; i++) {
		playerBullets[i].Initialize(engineCore_,"Player");
		playerBullets[i].SetMask(0x00000001);
	}
	for (int i = 0; i < kEnemyBullets; i++) {
		enemyBullets[i].Initialize(engineCore_,"Enemy");
		enemyBullets[i].SetMask(0x00000010);
	}

	for (int i = 0; i < kEnemies; i++) {
		enemies[i].Initialize(engineCore_);
	}

	enemies[0].Spawn({ 0.0f,0.0f,30.0f }, { 0.0f,0.0f,-2.0f });
	enemies[1].Spawn({ 1.5f,0.5f,30.0f }, { 0.0f,0.0f,-2.0f });
	enemies[2].Spawn({ -0.5f,-0.5f,30.0f }, { 0.0f,0.0f,-2.0f });

	timeCount_ = 0.0f;

	collisionManager_.Initalize();

	groundModel_.Initialize(engineCore_);
	groundModel_.LoadModel("Resources", "ground.obj", COORDINATESYSTEM_HAND_RIGHT);
	groundTransform_.translate.y = -2400.0f;
	groundTransform_.scale.x = 1400.0f;
	groundTransform_.scale.z = 1400.0f;
}

void GameScene::Update() {
	camera_.transform_.rotate.y += 0.001f;
	camera_.transform_.translate.z += 0.01f;
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
	
	
	player_.Update();
	player_.SetParent(camera_.GetWorldMatrix());

	if (player_.GetIsShot()) {
		for (int i = 0; i < kPlayerBullets; i++) {
			if (!playerBullets[i].GetIsActive()) {
				playerBullets[i].ShotBullet(
					Vector3::Transform(player_.transform_.translate,camera_.GetWorldMatrix()),
					Vector3::Transform({ 0.0f,0.0f,30.0f },Matrix4x4::Multiply( player_.GetRotateMatrix(), camera_.GetRotateMatrix())) , 120);
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
					enemyBullets[b].ShotBullet(enemies[i].transform_.translate, (player_.transform_.translate - enemies[i].transform_.translate).Normalize() * 10.0f, 6000);
					enemyBullets[b].transform_.rotate = Vector3::LookAt(enemyBullets[i].transform_.translate, player_.transform_.translate);
					break;
				}
			}
			enemies[i].SetIsShot(false);
		}
	}

	for (int i = 0; i < kPlayerBullets; i++) {
		if (playerBullets[i].GetIsActive()) {
			playerBullets[i].Update();
		}
	}
	for (int i = 0; i < kEnemyBullets; i++) {
		if (enemyBullets[i].GetIsActive()) {

			Vector3 toPlayer = player_.transform_.translate - enemyBullets[i].transform_.translate;
			enemyBullets[i].velocity_ = Vector3::Slerp(enemyBullets[i].velocity_.Normalize(), toPlayer.Normalize(), 0.1f) * 10.0f;

			enemyBullets[i].transform_.rotate = Vector3::LookAt(
				enemyBullets[i].transform_.translate.Normalize(),
				(enemyBullets[i].transform_.translate + enemyBullets[i].velocity_).Normalize());

			enemyBullets[i].Update();
		}
	}

	timeCount_ += engineCore_->GetDeltaTime();

	// 当たり判定
	std::list<Collider*> allColliders_;
	allColliders_.push_back(&player_);
	for (int i = 0; i < kPlayerBullets; i++) {
		if (playerBullets[i].GetIsActive()) {
			allColliders_.push_back(&playerBullets[i]);
		}
	}
	for (int i = 0; i < kEnemyBullets; i++) {
		if (enemyBullets[i].GetIsActive()) {
			allColliders_.push_back(&enemyBullets[i]);
		}
	}

	collisionManager_.Update(allColliders_);
}

void GameScene::Draw() {
	ImGui::Begin("GameScene");
	ImGui::Text("Time %.2f", timeCount_);
	ImGui::Text("isDebug: %s", isActiveDebugCamera_ ? "True" : "False");
	ImGui::Toggle("isActiveDebugCamera", &isActiveDebugCamera_);
	ImGui::DragFloat3("CameraTranslate", &camera_.transform_.translate.x,0.1f);
	ImGui::DragFloat3("CameraRotate", &camera_.transform_.rotate.x,0.01f);
	ImGui::DragFloat3("groundTransform", &groundTransform_.translate.x);
	ImGui::DragFloat3("groundScale", &groundTransform_.scale.x);
	if (ImGui::Button("Reset")) {
		Initialize();
	}
	ImGui::End();

	skyDome_.Draw(&camera_);
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

	groundModel_.transform_ = groundTransform_;
	groundModel_.Update();
	groundModel_.Draw(&camera_);
}

IScene* GameScene::GetNextScene() {
	return new TitleScene(engineCore_);
}