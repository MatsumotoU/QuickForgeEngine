#include "GameScene.h"
#include "TitleScene.h"
#include <algorithm>
#include "Utility/MyEasing.h"

GameScene::GameScene(EngineCore* engineCore) {
	engineCore_ = engineCore;
	input_ = engineCore_->GetInputManager();

#ifdef _DEBUG
	isActiveDebugCamera_ = false;
	debugCamera_.Initialize(engineCore_);
	debugCamera_.camera_.transform_.translate.z = -20.0f;
#endif // _DEBUG

	engineCore_->GetGraphRenderer()->SetCamera(&camera_);

	cameraT = 0.0f;
	cameraMoveSpeed_ = 0.1f;

	fpsCamera_.Initialize(engineCore_->GetWinApp());
}

GameScene::~GameScene() {
}

void GameScene::Initialize() {
	buillds_.Init(engineCore_);

	skyDome_.Initialize(engineCore_);
	camera_.Initialize(engineCore_->GetWinApp());

	isRequestedExit_ = false;

	player_.Initialize(engineCore_);
	player_.SetMask(0x00000001);

	for (int i = 0; i < kPlayerBullets; i++) {
		playerBullets[i].Initialize(engineCore_, "Player");
		playerBullets[i].SetMask(0x00000011);
		playerBullets[i].GetObjectData()["Attack"] = 1.0f;
		playerBullets[i].SetRadius(0.3f);
	}
	for (int i = 0; i < kEnemyBullets; i++) {
		enemyBullets[i].Initialize(engineCore_, "Enemy");
		enemyBullets[i].SetMask(0x10000010);
	}

	for (int i = 0; i < kEnemies; i++) {
		enemies[i].Initialize(engineCore_);
		enemies[i].SetMask(0x10000000);
	}

	timeCount_ = 0.0f;

	collisionManager_.Initalize();

	/*groundModel_.Initialize(engineCore_);
	groundModel_.LoadModel("Resources", "ground.obj", COORDINATESYSTEM_HAND_RIGHT);
	groundTransform_.translate.y = -2400.0f;
	groundTransform_.scale.x = 1400.0f;
	groundTransform_.scale.z = 1400.0f;*/

	lailPoints_.clear();
	lailPoints_.push_back({ 0.0f,0.0f,0.0f });
	lailPoints_.push_back({ 0.0f,0.0f,30.0f });
	lailPoints_.push_back({ 0.0f,0.0f,60.0f });
	lailPoints_.push_back({ 0.0f,0.0f,90.0f });

	isCartesian_ = true;

	isMoveLail_ = false;
	cameraMoveSpeed_ = 0.1f;

	std::vector<bool> isCalledSpone(kEnemies, false);

	std::string EnemyData = FileLoader::ReadFile("Resources/EnemyData/EnemySpone.txt");
	std::vector<std::string> sponeData = FileLoader::Split(EnemyData, ',');
	for (int i = 0; i < sponeData.size() / 5; i++) {

		for (int e = 0; e < kEnemies; e++) {
			if (isCalledSpone[e]) {
				continue;
			}
			Vector3 position = { std::stof(sponeData[i * 5]), std::stof(sponeData[i * 5 + 1]), std::stof(sponeData[i * 5 + 2]) };
			Vector3 vec = { 0.0f, 0.0f,0.0f };

			timedCalls_.push_back(
				new TimeCall(
					engineCore_,
					std::bind(&Enemy::Spawn, &enemies[e], position, vec, std::stoi(sponeData[i * 5 + 4])), std::stof(sponeData[i * 5 + 3])));
			isCalledSpone[e] = true;
			break;
		}
	}
	isFpsCamera_ = false;
	reticle_.Initialize(engineCore_);
	reticle_.SetPlayer(&player_);

	lockOn_.Initialize();
	isLockOn_ = false;

	shieldBar_.Initialize(engineCore_);
	playerHitPoint_.Initialize(engineCore_);
}

void GameScene::Update() {
	buillds_.Update();

	lockOn_.ResetTargets();

	/*groundModel_.transform_ = groundTransform_;
	groundModel_.Update();*/

	for (TimeCall* timedCall : timedCalls_) {
		timedCall->Update();
	}

	timedCalls_.remove_if([](TimeCall* call) {
		if (call->IsFinished()) {
			delete call;
			return true;
		}
		return false;
		});


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

	if (isMoveLail_) {
		if (cameraT < 1.0f) {
			cameraT += engineCore_->GetDeltaTime() * cameraMoveSpeed_;
		} else {
			cameraT = 0.0f;
		}
	}

	Vector3 target = Vector3::CatmullRom(lailPoints_, cameraT) + player_.transform_.translate.Normalize() * 0.5f;
	Eas::SimpleEaseIn(&camera_.transform_.translate.x, target.x, 0.1f);
	Eas::SimpleEaseIn(&camera_.transform_.translate.y, target.y, 0.1f);
	Eas::SimpleEaseIn(&camera_.transform_.translate.z, target.z, 0.1f);

	camera_.transform_.rotate = -Vector3::LookAt(
		Vector3::CatmullRom(lailPoints_, cameraT + 0.01f),
		Vector3::CatmullRom(lailPoints_, cameraT));

	player_.Update();
	//player_.SetParent(camera_.GetWorldMatrix());

	if (player_.GetIsShot()) {
		for (int i = 0; i < kPlayerBullets; i++) {
			if (!playerBullets[i].GetIsActive()) {
				playerBullets[i].ShotBullet(
					Vector3::Transform(player_.transform_.translate, camera_.GetWorldMatrix()),
					Vector3::Transform({ 0.0f,0.0f,60.0f }, Matrix4x4::Multiply(player_.GetRotateMatrix(), camera_.GetRotateMatrix())), 120);
				break;
			}
		}
		player_.SetIsShot(false);
	}

	for (int i = 0; i < kEnemies; i++) {
		enemies[i].Update();
		if (enemies[i].GetIsActive()) {
			lockOn_.AddTargetPosition(enemies[i].GetWorldPosition());
		}

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

			Vector3 toPlayer = player_.GetWorldPosition() - enemyBullets[i].transform_.translate;
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
	for (int i = 0; i < kEnemies; i++) {
		if (enemies[i].GetIsActive()) {
			allColliders_.push_back(&enemies[i]);
		}
	}
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

	if (isFpsCamera_) {
		fpsCamera_.Update();
		fpsCamera_.transform_.translate = player_.GetWorldPosition();
		fpsCamera_.transform_.rotate = camera_.transform_.rotate;
		camera_ = fpsCamera_;
	}

	reticle_.Update();
	if (isLockOn_) {
		lockOn_.SetReticlePosition(reticle_.GetReticleWorldPos());
		if (Vector3::Dot(player_.GetDir().Normalize(), (player_.GetWorldPosition() - lockOn_.GetLockPosition(&camera_)).Normalize()) < 0.0f) {
			reticle_.model_.worldMatrix_ =
				Matrix4x4::MakeAffineMatrix(reticle_.transform_.scale, reticle_.transform_.rotate, lockOn_.GetLockPosition(&camera_));
		}

	}

	// プレイヤーの反撃処理
	if (player_.GetIsRevenge()) {
		player_.SetIsRevenge(false);

		// レティクルのスクリーン座標を取得
		Vector3 reticleScreenPos = camera_.GetWorldToScreenPos(
			reticle_.GetReticleWorldPos());
		Vector3 reticleTopScreenPos = camera_.GetWorldToScreenPos(
			reticle_.GetTopPos());

		reticleScreenPos.z = 0.0f; // Z座標を無視して2D座標に変換
		reticleTopScreenPos.z = 0.0f; // Z座標を無視して2D座標に変換

		Vector3 reticleRadiusVec = reticleScreenPos - reticleTopScreenPos;
		float reticleRadius = reticleRadiusVec.Length();

		for (int i = 0; i < kEnemies; i++) {
			if (!enemies[i].GetIsActive()) continue;

			// 敵のワールド座標をスクリーン座標に変換
			Vector3 enemyScreenPos = camera_.GetWorldToScreenPos(
				enemies[i].GetWorldPosition());
			enemyScreenPos.z = 0.0f;

			// 2D距離で判定
			float dx = reticleScreenPos.x - enemyScreenPos.x;
			float dy = reticleScreenPos.y - enemyScreenPos.y;
			Vector3 diff = { dx, dy, 0.0f };

			if (diff.Length() < reticleRadius) {
				if (enemies[i].GetIsShield()) {
					for (int e = 0; e < 16; e++) {
						for (int b = 0; b < kEnemyBullets; b++) {
							if (!enemyBullets[b].GetIsActive()) {
								// 角度を計算（0～2πを16分割）
								float angle = (2.0f * 3.14159265f * e) / 16.0f;
								// xy平面上の単位ベクトル
								Vector3 dir = { std::cos(angle), std::sin(angle), 0.0f };
								// 速度ベクトルを作成
								Vector3 velocity = dir * 60.0f;
								enemyBullets[b].ShotBullet(enemies[i].transform_.translate, velocity, 6000);
								break;
							}
						}
					}
				} else {
					enemies[i].HitRevenge(player_.GetShieldLevel());
				}
			}
		}
	}

	playerHitPoint_.SetHitPoint(player_.GetHitPoint());
	playerHitPoint_.Update();

	shieldBar_.SetShieldPoint(player_.GetShieldPoint());
	shieldBar_.Update();

	if (!player_.GetIsActive()) {
		isRequestedExit_ = true;
	}
}

void GameScene::Draw() {
	buillds_.Draw(&camera_);
	playerHitPoint_.Draw(&camera_);
	shieldBar_.Draw(&camera_);
#ifdef _DEBUG
	debugCamera_.DrawImGui();
#endif // _DEBUG

	
	reticle_.Draw(&camera_);
#ifdef _DEBUG
	ImGui::Begin("LockOn");
	if (ImGui::Button("isLockOn")) {
		isLockOn_ = !isLockOn_;
	}
	Vector3 r = lockOn_.GetLockPosition(&camera_);
	ImGui::Text("isLockOn: %d", isLockOn_);
	ImGui::Text("%f,%f,%f", r.x, r.y, r.z);
	ImGui::Text("length: %f", (lockOn_.GetLockPosition(&camera_) - player_.GetWorldPosition()).Length());
	ImGui::Text("Dot: %f", Vector3::Dot(player_.GetDir().Normalize(), (player_.GetWorldPosition() - lockOn_.GetLockPosition(&camera_)).Normalize()));
	ImGui::End();

	ImGui::Begin("Enemy");
	for (int i = 0; i < kEnemies; i++) {
		if (!enemies[i].GetIsActive()) {
			continue;
		}
		ImGui::Text("Enemy %d", i);
		ImGui::Text("MoveType %d", enemies[i].moveType_);
		ImGui::DragFloat3(("Position" + std::to_string(i)).c_str(), &enemies[i].transform_.translate.x, 0.1f);
		ImGui::DragFloat3(("Rotation" + std::to_string(i)).c_str(), &enemies[i].transform_.rotate.x, 0.01f);
	}
	ImGui::End();

	ImGui::Begin("Lail");
	if (ImGui::Button("Add Point")) {
		lailPoints_.push_back({ 0.0f,0.0f,0.0f });
	}
	ImGui::SameLine();
	if (ImGui::Button("Delete Points")) {
		if (lailPoints_.size() > 4) {
			lailPoints_.erase(lailPoints_.end());
		}
	}
	ImGui::Text("Lail Points");
	for (int i = 0; i < lailPoints_.size(); i++) {
		ImGui::DragFloat3(("Point" + std::to_string(i)).c_str(), &lailPoints_[i].x, 0.1f);
	}

	ImGui::End();

	ImGui::Begin("GameScene");
	if (ImGui::Button("LockOn")) {
		isLockOn_ = !isLockOn_;
	}
	if (ImGui::Button("MoveLail")) {
		isMoveLail_ = !isMoveLail_;
	}
	if (ImGui::Button("FpsCamera")) {
		isFpsCamera_ = !isFpsCamera_;
	}
	ImGui::DragFloat("CameraMoveSpeed", &cameraMoveSpeed_, 0.01f, 0.01f, 1.0f);
	ImGui::DragFloat("CameraT", &cameraT, 0.01f, 0.0f, 1.0f);
	ImGui::Text("Time %.2f", timeCount_);
	ImGui::Text("isDebug: %s", isActiveDebugCamera_ ? "True" : "False");
	ImGui::Toggle("isActiveDebugCamera", &isActiveDebugCamera_);
	ImGui::DragFloat3("CameraTranslate", &camera_.transform_.translate.x, 0.1f);
	ImGui::DragFloat3("CameraRotate", &camera_.transform_.rotate.x, 0.01f);
	ImGui::DragFloat3("groundTransform", &groundTransform_.translate.x);
	ImGui::DragFloat3("groundScale", &groundTransform_.scale.x);
	if (ImGui::Button("Reset")) {
		Initialize();
	}
	ImGui::End();
#endif // _DEBUG

	skyDome_.Draw(&camera_);
	if (!isFpsCamera_) {
		player_.Draw(&camera_);
	}

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

	//groundModel_.Draw(&camera_);

	Vector3 p0 = player_.GetWorldPosition();
	p0.z += 60.0f;
	engineCore_->GetGraphRenderer()->DrawLine(player_.GetWorldPosition(), p0);
}

IScene* GameScene::GetNextScene() {
	return new TitleScene(engineCore_);
}