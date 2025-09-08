#include "Enemy.h"
#include "Block.h"

#include "Utility/MyEasing.h"

void Enemy::Initialize(EngineCore* engineCore, Camera* camera) {
	engineCore_ = engineCore;
	camera_ = camera;
	model_ = std::make_unique<Model>(engineCore, camera);
	model_->LoadModel("Resources/mole", "mole_fall.obj", COORDINATESYSTEM_HAND_RIGHT);

	accelerationDamping_ = 0.95f;
	velocityDamping_ = 0.98f;

	isAlive_ = true;
	isCanMove_ = false;
	isCanShot_ = false;
	isMoving_ = false;
	isReqestBuilding_ = false;
	isGrounded_ = false;

	moveTimer_ = 0.0f;
	maxMoveTimer_ = 1.0f;
	shotPower_ = 6.0f;
	moveDir_ = { 0.0f,0.0f };

	model_->transform_.translate.y = 1.0f;
	alpha_ = 0.5f;
	model_->SetColor({ 1.0f,0.0f,1.0f,alpha_ });

	for (int i = 0; i < kCheckCount_; i++) {
		totalEvaluationValue_[i] = 0;
		lifeEvaluationValue_[i] = 0;
		attackEvaluationValue_[i] = 0;
		uniqeEvaluationValue_[i] = 0;
	}

	for (int i = 0; i < kCheckCount_; ++i) {
		float angle = (2.0f * 3.1415f * i) / kCheckCount_;
		directionTable_[i] = Vector2(std::cos(angle), std::sin(angle));
	}

	aiWeight_.life = 1.0f;
	aiWeight_.attack = 0.0f;
	aiWeight_.uniqe = 0.0f;

	aiLevel_ = 1;
	delayTimer_ = 2.0f;

	selectedDirIndex_ = 0;
	maxSelect_ = 0;

	isSelectedDir_ = false;
	selectAnimIndex_ = 0;
	timer_ = 0.0f;
}

void Enemy::Update() {
	float deltaTime = engineCore_->GetDeltaTime();
	timer_ += deltaTime;

	// moveDir_の方向にモデルを向かせる
	if (moveDir_.Length() > 0.0f) {
		MyEasing::SimpleEaseIn(&model_->transform_.rotate.y, -std::atan2(moveDir_.y, moveDir_.x) + 3.14f * 0.5f, 0.2f);
	}

	if (isCanMove_ && !isJumping_ && !isGrounded_) {
		model_->transform_.scale.x = 1.0f + sinf(timer_) * 0.1f;
		model_->transform_.scale.y = 1.0f + sinf(timer_) * 0.1f;
		model_->transform_.scale.z = 1.0f + sinf(timer_) * 0.1f;

		if (!isSelectedDir_) {
			InitEvaluationValue();
			LifeEvaluation();
			AttackEvaluation();
			UniqeEvaluation();
			TotalEvaluation();
			SortDirTableValue();
			SelectedDir();
			ShotRandomDir();
			isSelectedDir_ = true;
			selectAnimIndex_ = 0;
		}
		
		if (isSelectedDir_) {
			if (delayTimer_ > 0.0f) {
				delayTimer_ -= deltaTime;
			} else {
				delayTimer_ = 0.0f;
				if (selectAnimIndex_ < selectedDirIndex_) {
					moveDir_ = directionTable_[dirTableIndexTable_[selectAnimIndex_]];
					selectAnimIndex_++;

					if (engineCore_->GetXInputController()->GetPressButton(XINPUT_GAMEPAD_RIGHT_SHOULDER, 0) || engineCore_->GetXInputController()->GetPressButton(XINPUT_GAMEPAD_A, 0)) {
						delayTimer_ = 0.05f;
					} else {
						delayTimer_ = 0.3f + static_cast<float>(std::rand()) / RAND_MAX * 0.2f;
					}
					
				} else {
					moveDir_ = shotDir_;
					float currentAngle = model_->transform_.rotate.y;
					Vector2 currentDir(std::sin(currentAngle), std::cos(currentAngle));
					if ((currentDir - shotDir_).Length() < 0.1f) {
						Shot(shotDir_);
					}
				}
			}
		}
	} else {
		model_->transform_.scale.x = 1.0f;
		model_->transform_.scale.y = 1.0f;
		model_->transform_.scale.z = 1.0f;
	}

	// 動いてる最中の処理
	if (isMoving_) {
		MyEasing::SimpleEaseIn(&model_->transform_.translate.y, 0.0f, 0.3f);

		if (moveTimer_ > 0.0f) {
			model_->transform_.translate.x += moveDir_.x * shotPower_ * deltaTime;
			model_->transform_.translate.z += moveDir_.y * shotPower_ * deltaTime;
			moveTimer_ -= deltaTime;
		} else {
			isMoving_ = false;
			isReqestBuilding_ = true;
			model_->transform_.translate.y = 1.0f;
		}
	}

	// 移動の処理
	model_->transform_.translate += velocity_ * deltaTime;
	velocity_ = velocity_ * velocityDamping_;

	if (velocity_.y > -5.0f) {
		velocity_.y -= 9.81f * deltaTime;
	}

	if (model_->transform_.translate.y <= 1.0f) {
		if (!isMoving_) {
			model_->transform_.translate.y = 1.0f;
		}
		velocity_.y = 0.0f;

		if (isJumping_) {
			isJumping_ = false;
			isGrounded_ = true;
		}
	}

	model_->SetColor({ 1.0f,0.0f,1.0f,alpha_ });
	model_->Update();
}

void Enemy::Draw() {
	ImGui::Begin("Enemy");

	int aiLevel = aiLevel_;
	ImGui::DragInt("AiLevel",&aiLevel);
	aiLevel_ = std::clamp(aiLevel, 1, 100);
	ImGui::DragFloat3("AIWeight", &aiWeight_.life, 0.1f, 0.0f, 1.0f);
	ImGui::Text("SelectIndex: %d MaxSelected: %d", selectedDirIndex_, maxSelect_);

	ImGui::Text("DelayTime: %f", delayTimer_);
	ImGui::DragFloat2("moveDir", &moveDir_.x, 0.1f);
	ImGui::DragFloat3("Velocity", &velocity_.x, 0.1f);
	ImGui::DragFloat3("Position", &model_->transform_.translate.x, 0.1f);
	ImGui::DragFloat("ShotPower", &shotPower_, 0.01f, 0.0f, 10.0f);
	ImGui::Checkbox("isCanMove", &isCanMove_);
	ImGui::Checkbox("isCanShot", &isCanShot_);
	ImGui::Checkbox("isMoving", &isMoving_);
	ImGui::Checkbox("isReqestBuilding", &isReqestBuilding_);
	ImGui::Text("MoveDir:%f,%f", moveDir_.x, moveDir_.y);
	ImGui::Text("MoveTimer:%f", moveTimer_);

	Vector2 leftStick = engineCore_->GetXInputController()->GetLeftStick(0);
	ImGui::Text("%f,%f", leftStick.x, leftStick.y);

	ImGui::End();
	model_->Draw();
}

void Enemy::Shot(Vector2& dir) {
	if (isCanMove_) {
		moveDir_ = dir;
		moveTimer_ = maxMoveTimer_;
		isCanMove_ = false;
		isCanShot_ = false;
		isMoving_ = true;
		isSelectedDir_ = false;
	}
}

void Enemy::InitEvaluationValue() {
	for (int i = 0; i < kCheckCount_; i++) {
		totalEvaluationValue_[i] = 0;
		lifeEvaluationValue_[i] = 0;
		attackEvaluationValue_[i] = 0;
		uniqeEvaluationValue_[i] = 0;
	}
}

void Enemy::TotalEvaluation() {
	for (int i = 0; i < kCheckCount_; i++) {
		totalEvaluationValue_[i] = static_cast<int>(
			static_cast<float>(lifeEvaluationValue_[i]) * aiWeight_.life +
			static_cast<float>(attackEvaluationValue_[i]) * aiWeight_.attack +
			static_cast<float>(uniqeEvaluationValue_[i]) * aiWeight_.uniqe
			);
	}
}

void Enemy::SortDirTableValue() {
	// インデックス配列を初期化
	for (int i = 0; i < kCheckCount_; i++) {
		dirTableIndexTable_[i] = i;
	}
	// 評価値降順でインデックスをソート
	std::sort(dirTableIndexTable_.begin(), dirTableIndexTable_.end(),
		[this](int a, int b) {
			return totalEvaluationValue_[a] > totalEvaluationValue_[b];
		});
}

void Enemy::SelectedDir() {
	selectedDirIndexTable_.clear();
	assert(aiLevel_ > 0 && aiLevel_ <= 100);
	uint32_t maxSelect = static_cast<int>(static_cast<float>(kCheckCount_) * (static_cast<float>(100 - aiLevel_) / 100.0f));
	maxSelect = std::max(maxSelect, 1u); // 最低1つは選択肢を残す
	maxSelect_ = maxSelect;

	for (uint32_t i = 0; i < maxSelect; i++) {
		selectedDirIndexTable_.push_back(dirTableIndexTable_[i]);
	}
}

void Enemy::ShotRandomDir() {
	// ランダムに選択
	int randomIndex = rand() % selectedDirIndexTable_.size();
	selectedDirIndex_ = randomIndex;
	shotDir_ = directionTable_[selectedDirIndexTable_[randomIndex]];
}

void Enemy::LifeEvaluation() {
	// ゲーム盤の中央座標
	const float mapCenterX = static_cast<float>(wallMap_[0].size()) * kBlockSize * 0.5f;
	const float mapCenterY = static_cast<float>(wallMap_->size()) * kBlockSize * 0.5f;

	for (int i = 0; i < kCheckCount_; i++) {
		// 予測移動（directionTable_[i]方向にGetMoveTimer()分進む）
		Vector2 startPos = { model_->transform_.translate.x, model_->transform_.translate.z };
		Vector2 dir = directionTable_[i].Normalize();

		predictionLine_.Init();
		predictionLine_.Scan({ startPos.x, 1.0f, startPos.y }, dir, static_cast<int>(GetMoveTimer()), *wallMap_, kBlockSize);
		const auto& points = predictionLine_.GetLinePoints();

		bool isDead = false;
		Vector2 lastPos = startPos;
		// 予測線の各頂点をチェック
		for (const auto& pos : points) {
			int mapX = static_cast<int>(pos.x / kBlockSize);
			int mapY = static_cast<int>(pos.y / kBlockSize);
			// 盤外判定
			if (mapX < 0 || mapX >= static_cast<int>(wallMap_[0].size()) ||
				mapY < 0 || mapY >= static_cast<int>(wallMap_->size())) {
				isDead = true;
				lastPos = pos;
				break;
			}
			// 死ぬ判定（床がない）
			if (floorMap_ && mapY >= 0 && mapY < static_cast<int>(floorMap_->size()) &&
				mapX >= 0 && mapX < static_cast<int>((*floorMap_)[mapY].size()) &&
				(*floorMap_)[mapY][mapX] == 0) {
				isDead = true;
				lastPos = pos;
				break;
			}
			lastPos = pos;
		}

		if (isDead) {
			lifeEvaluationValue_[i] = -10; // 死ぬ
		} else {
			lifeEvaluationValue_[i] = 5; // 盤外に落ちない
			// 中央に近いほど高評価
			float distToCenter = std::sqrt(
				(lastPos.x - mapCenterX) * (lastPos.x - mapCenterX) +
				(lastPos.y - mapCenterY) * (lastPos.y - mapCenterY)
			);
			// 距離が近いほど高評価（例: 1点 - 距離補正）
			lifeEvaluationValue_[i] += static_cast<int>(1.0f - distToCenter * 0.01f);
		}
	}
}

void Enemy::AttackEvaluation() {
	// プレイヤー座標（事前にplayerPos_へセットされている前提）
	Vector2 playerPos = playerPos_;

	// ゲーム盤の中央座標
	const float mapCenterX = static_cast<float>(wallMap_[0].size()) * kBlockSize * 0.5f;
	const float mapCenterY = static_cast<float>(wallMap_->size()) * kBlockSize * 0.5f;

	for (int i = 0; i < kCheckCount_; i++) {
		// 予測移動（directionTable_[i]方向にGetMoveTimer()分進む）
		Vector2 startPos = { model_->transform_.translate.x, model_->transform_.translate.z };
		Vector2 dir = directionTable_[i].Normalize();

		predictionLine_.Init();
		predictionLine_.Scan({ startPos.x, 1.0f, startPos.y }, dir, static_cast<int>(GetMoveTimer()), *wallMap_, kBlockSize);
		const auto& points = predictionLine_.GetLinePoints();

		// 最終到達点
		Vector2 lastPos = startPos;
		if (!points.empty()) {
			lastPos = points.back();
		}

		// 1. プレイヤーより中央に近いほど高評価
		float distToCenter = std::sqrt(
			(lastPos.x - mapCenterX) * (lastPos.x - mapCenterX) +
			(lastPos.y - mapCenterY) * (lastPos.y - mapCenterY)
		);
		float playerDistToCenter = std::sqrt(
			(playerPos.x - mapCenterX) * (playerPos.x - mapCenterX) +
			(playerPos.y - mapCenterY) * (playerPos.y - mapCenterY)
		);
		if (distToCenter < playerDistToCenter) {
			attackEvaluationValue_[i] += 3; // プレイヤーより中央に近い
		}

		// 2. プレイヤーより広い場所にいるほど高評価
		int mySpace = 0;
		int playerSpace = 0;
		// 周囲8マス＋自分のマス
		for (int dy = -1; dy <= 1; ++dy) {
			for (int dx = -1; dx <= 1; ++dx) {
				int nx = static_cast<int>(lastPos.x / kBlockSize) + dx;
				int ny = static_cast<int>(lastPos.y / kBlockSize) + dy;
				if (floorMap_ && wallMap_ &&
					ny >= 0 && ny < static_cast<int>(floorMap_->size()) &&
					nx >= 0 && nx < static_cast<int>((*floorMap_)[ny].size()) &&
					ny < static_cast<int>(wallMap_->size()) &&
					nx < static_cast<int>((*wallMap_)[ny].size()) &&
					(*floorMap_)[ny][nx] != 0) {
					mySpace++;
				}
				int px = static_cast<int>(playerPos.x / kBlockSize) + dx;
				int py = static_cast<int>(playerPos.y / kBlockSize) + dy;
				if (floorMap_ && wallMap_ &&
					py >= 0 && py < static_cast<int>(floorMap_->size()) &&
					px >= 0 && px < static_cast<int>((*floorMap_)[py].size()) &&
					py < static_cast<int>(wallMap_->size()) &&
					px < static_cast<int>((*wallMap_)[py].size()) &&
					(*floorMap_)[py][px] != 0) {
					playerSpace++;
				}
			}
		}
		if (mySpace > playerSpace) {
			attackEvaluationValue_[i] += 2; // プレイヤーより広い場所
		}
	}
}

void Enemy::UniqeEvaluation() {
	uniqeEvaluationValue_.fill(0);
}
