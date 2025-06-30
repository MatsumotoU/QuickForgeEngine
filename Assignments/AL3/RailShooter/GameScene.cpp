#include "GameScene.h"
#include "TitleScene.h"

GameScene::GameScene(EngineCore* engineCore) {
	engineCore_ = engineCore;
	input_ = engineCore_->GetInputManager();
	isActiveDebugCamera_ = false;

#ifdef _DEBUG
	debugCamera_.Initialize(engineCore_);
	debugCamera_.camera_.transform_.translate.z = -20.0f;
#endif // _DEBUG

	engineCore_->GetGraphRenderer()->SetCamera(&camera_);

	cameraT = 0.0f;
	cameraMoveSpeed_ = 0.1f;

	fpsCamera_.Initialize(engineCore_->GetWinApp());

	audioHandle[0] = engineCore_->LoadSoundData("Resources/", "mokugyo.wav");
	audioHandle[1] = engineCore_->LoadSoundData("Resources/", "mokugyo.wav");
	audioHandle[2] = engineCore_->LoadSoundData("Resources/", "mono48kHz.wav");

	engineCore_->GetAudioSourceBinder()->CreateSourceVoice("mokugyo1", audioHandle[0]);
	engineCore_->GetAudioSourceBinder()->CreateSourceVoice("mokugyo2", audioHandle[1]);
	engineCore_->GetAudioSourceBinder()->CreateSourceVoice("mono48kHz", audioHandle[2]);
}

GameScene::~GameScene() {
}

void GameScene::Initialize() {
	skyDome_.Initialize(engineCore_);
	camera_.Initialize(engineCore_->GetWinApp());

	isRequestedExit_ = false;

	player_.Initialize(engineCore_);
	player_.SetMask(0x00000001);

	for (int i = 0; i < kPlayerBullets; i++) {
		playerBullets[i].Initialize(engineCore_, "Player");
		playerBullets[i].SetMask(0x00000001);
	}
	for (int i = 0; i < kEnemyBullets; i++) {
		enemyBullets[i].Initialize(engineCore_, "Enemy");
		enemyBullets[i].SetMask(0x00000010);
	}

	for (int i = 0; i < kEnemies; i++) {
		enemies[i].Initialize(engineCore_);
	}

	timeCount_ = 0.0f;

	collisionManager_.Initalize();

	groundModel_.Initialize(engineCore_);
	groundModel_.LoadModel("Resources", "ground.obj", COORDINATESYSTEM_HAND_RIGHT);
	groundTransform_.translate.y = -2400.0f;
	groundTransform_.scale.x = 1400.0f;
	groundTransform_.scale.z = 1400.0f;

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
			Vector3 vec = { 0.0f, 0.0f, -5.0f };

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
}

void GameScene::Update() {
	lockOn_.ResetTargets();

	groundModel_.transform_ = groundTransform_;
	groundModel_.Update();

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

	camera_.transform_.translate = Vector3::CatmullRom(lailPoints_, cameraT);
	camera_.transform_.rotate = -Vector3::LookAt(
		Vector3::CatmullRom(lailPoints_, cameraT + 0.01f),
		Vector3::CatmullRom(lailPoints_, cameraT));

	player_.Update();
	player_.SetParent(camera_.GetWorldMatrix());


	if (player_.GetIsShot()) {
		for (int i = 0; i < kPlayerBullets; i++) {
			if (!playerBullets[i].GetIsActive()) {
				playerBullets[i].ShotBullet(
					Vector3::Transform(player_.transform_.translate, camera_.GetWorldMatrix()),
					Vector3::Transform({ 0.0f,0.0f,30.0f }, Matrix4x4::Multiply(player_.GetRotateMatrix(), camera_.GetRotateMatrix())), 120);
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
}

void GameScene::Draw() {

	debugCamera_.DrawImGui();
	reticle_.Draw(&camera_);

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
		engineCore_->GetAudioPlayer()->PlayAudio(audioHandle[0], "mokugyo1");
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


	groundModel_.Draw(&camera_);

	for (float i = 0.0f; i < 1.0f; i += 0.01f) {
		Vector3 p0 = Vector3::CatmullRom(lailPoints_, i);
		Vector3 p1 = Vector3::CatmullRom(lailPoints_, i + 0.01f);
		engineCore_->GetGraphRenderer()->DrawLine(p0, p1);
	}
}


IScene* GameScene::GetNextScene() {
	return new TitleScene(engineCore_);
}