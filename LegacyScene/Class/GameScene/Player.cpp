#include "Player.h"
#include "Utility/MyEasing.h"
#include "Block.h"
#include "Collition/MapChipCollider.h"

void Player::Initialize(EngineCore* engineCore, Camera* camera) {
	engineCore_ = engineCore;
	camera_ = camera;
	model_ = std::make_unique<Model>(engineCore, camera);
	model_->LoadModel("Resources/mole", "mole.obj", COORDINATESYSTEM_HAND_RIGHT);

	accelerationDamping_ = 0.95f;
	velocityDamping_ = 0.98f;

	isAlive_ = true;
	isCanMove_ = true;
	isCanShot_ = false;
	isMoving_ = false;
	isReqestBuilding_ = false;
	isGrounded_ = false;

	moveTimer_ = 0.0f;
	maxMoveTimer_ = 1.0f;
	shotPower_ = 6.0f;
	moveDir_ = { 0.0f,0.0f };

	model_->transform_.translate.y = 1.0f;
	alpha_ = 1.0f;
	model_->SetColor({ 0.0f,1.0f,0.0f,alpha_ });
	color_ = { 0.0f,1.0f,0.0f,1.0f };

	isClicked_ = false;
	clickStartPos_ = { 0.0f,0.0f };

	timer_ = 0.0f;

	predictionLine_.Init();

	actionWeight_ = { 1.0f,0.0f,0.0f };

	lifeEvaluationValue_ = 0;
	attackEvaluationValue_ = 0;
	uniqeEvaluationValue_ = 0;
	totalEvaluationValue_ = 0;

	totalActionWeight_ = { 0.0f,0.0f,0.0f };

	chageTimer_ = 0.0f;
}

void Player::Update() {
	float deltaTime = engineCore_->GetDeltaTime();
	timer_ += deltaTime;

	// moveDir_の方向にモデルを向かせる
	if (moveDir_.Length() > 0.0f) {
		model_->transform_.rotate.y = -std::atan2(moveDir_.y, moveDir_.x) + 3.14f * 0.5f;
	}

	// はじきの処理
	if (isCanMove_) {
		model_->transform_.scale.x = 1.0f + sinf(timer_ + (chageTimer_ * 10.0f)) * 0.1f;
		model_->transform_.scale.y = 1.0f + sinf(timer_ + (chageTimer_ * 10.0f)) * 0.1f;
		model_->transform_.scale.z = 1.0f + sinf(timer_ + (chageTimer_ * 10.0f)) * 0.1f;

		MouseControl();
		ControllerControl();
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
			velocity_.x = 0.0f;
			velocity_.z = 0.0f;

			isJumping_ = false;
			isGrounded_ = true;
		}
	}

	model_->SetColor({ 0.0f,1.0f,0.0f,alpha_ });
	model_->Update();
}

void Player::Draw() {
#ifdef _DEBUG
	ImGui::Begin("Player");

	ImGui::DragFloat3("ActionWeight", &actionWeight_.x, 0.01f, 0.0f, 1.0f);
	ImGui::DragFloat3("TotalActionWeight", &totalActionWeight_.x, 0.01f, 0.0f, 1.0f);
	ImGui::Text("Life:%d Attack:%d Uniqe:%d", lifeEvaluationValue_, attackEvaluationValue_, uniqeEvaluationValue_);
	ImGui::Text("Total:%d", totalEvaluationValue_);

	ImGui::End();
#endif // _DEBUG

	model_->Draw();
}

void Player::ResetActionWeight() {
	totalActionWeight_ = { 0.0f,0.0f,0.0f };
	actionWeight_ = { 1.0f,0.0f,0.0f };
}

Vector3 Player::GetActionWeight() const {
	return totalActionWeight_;
}

void Player::MouseControl() {
	// マウス操作
	if (engineCore_->GetInputManager()->mouse_.GetTrigger(0)) {
		isClicked_ = true;
		clickStartPos_ = engineCore_->GetInputManager()->mouse_.mouseScreenPos_;
	}

	if (engineCore_->GetInputManager()->mouse_.GetPress(0)) {
		// ドラッグ中の処理
		if (isClicked_) {
			Vector2 currentMousePos = engineCore_->GetInputManager()->mouse_.mouseScreenPos_;
			Vector2 dragVector = clickStartPos_ - currentMousePos;
			dragVector.x *= -1.0f;

			moveDir_ = -dragVector.Normalize();
		}

		chageTimer_ += engineCore_->GetDeltaTime();
	}

	if (engineCore_->GetInputManager()->mouse_.GetRelease(0)) {
		if (isClicked_) {
			isClicked_ = false;
			Vector2 clickEndPos = engineCore_->GetInputManager()->mouse_.mouseScreenPos_;
			Vector2 dragVector = clickStartPos_ - clickEndPos;
			if (dragVector.Length() >= 5.0f && chageTimer_ >= 0.5f) {
				isCanShot_ = true;
				moveTimer_ = maxMoveTimer_;

				if (isCanShot_) {
					Shot();
					chageTimer_ = 0.0f;
				}
			}
		}
	}
}

void Player::ControllerControl() {
	// まだ動いてないときにスティックを倒したら動けるようにする
	Vector2 leftStick = engineCore_->GetXInputController()->GetLeftStick(0);
	if (leftStick.Length() >= 3.0f) {
		if (chageTimer_ >= 0.5f) {
			isCanShot_ = true;
			moveTimer_ = maxMoveTimer_;
		}
		moveDir_ = -leftStick.Normalize();
		chageTimer_ += engineCore_->GetDeltaTime();
		
	} else {
		if (isCanShot_) {
			Shot();
			chageTimer_ = 0.0f;
		}
	}

	// ボタンでも発射できる
	if (engineCore_->GetInputManager()->keyboard_.GetTrigger(DIK_SPACE) || engineCore_->GetXInputController()->GetTriggerButton(XINPUT_GAMEPAD_A, 0)) {
		if (isCanShot_) {
			moveDir_ = -leftStick.Normalize();
			Shot();
			chageTimer_ = 0.0f;
		}
	}
}

void Player::Shot() {
	isCanMove_ = false;
	isCanShot_ = false;
	isMoving_ = true;

	predictionLine_.Init();
	predictionLine_.Scan(
		{ model_->transform_.translate.x, 1.0f, model_->transform_.translate.z },
		moveDir_, static_cast<int>(maxMoveTimer_), *wallMap_, 1.0f);

	lifeEvaluationValue_ = 0;
	attackEvaluationValue_ = 0;
	uniqeEvaluationValue_ = 0;

	ScanMapCenter();
	LifeEvaluation();
	AttackEvaluation();
	UniqeEvaluation();
	totalEvaluationValue_ = lifeEvaluationValue_ + attackEvaluationValue_ + uniqeEvaluationValue_;

	actionWeight_ = {
		static_cast<float>(lifeEvaluationValue_),
		static_cast<float>(attackEvaluationValue_),
		static_cast<float>(uniqeEvaluationValue_)
	};

	actionWeight_ = actionWeight_.Normalize();
	totalActionWeight_ += actionWeight_;
	if (totalActionWeight_.x < 0.0f) {
		totalActionWeight_.x = 0.0f;
	}
	if (totalActionWeight_.y < 0.0f) {
		totalActionWeight_.y = 0.0f;
	}
	if (totalActionWeight_.z < 0.0f) {
		totalActionWeight_.z = 0.0f;
	}

	if (totalActionWeight_.Length() == 0.0f) {
		totalActionWeight_ = { 0.5f,0.5f,0.5f };
	}

	totalActionWeight_ = totalActionWeight_.Normalize();
}

void Player::ScanMapCenter() {
	// 穴リスト作成
	std::vector<std::pair<int, int>> holeList;
	for (size_t y = 0; y < floorMap_->size(); ++y) {
		for (size_t x = 0; x < (*floorMap_)[y].size(); ++x) {
			if ((*floorMap_)[y][x] == 0) {
				holeList.emplace_back(static_cast<int>(x), static_cast<int>(y));
			}
		}
	}

	// 床ブロックごとに穴からの最短距離を計算
	struct BlockInfo {
		int x, y;
		float minDistToHole;
	};
	std::vector<BlockInfo> blockInfos;
	for (size_t y = 0; y < floorMap_->size(); ++y) {
		for (size_t x = 0; x < (*floorMap_)[y].size(); ++x) {
			if ((*floorMap_)[y][x] != 0) {
				float minDistToHole = std::numeric_limits<float>::max();
				for (const auto& hole : holeList) {
					float dist = static_cast<float>(
						std::sqrt((static_cast<int>(x) - hole.first) * (static_cast<int>(x) - hole.first) +
							(static_cast<int>(y) - hole.second) * (static_cast<int>(y) - hole.second)));

					if (dist < minDistToHole) {
						minDistToHole = dist;
					}
				}
				blockInfos.push_back({ static_cast<int>(x), static_cast<int>(y), minDistToHole });
			}
		}
	}

	// 穴から最も遠い順にソート
	std::sort(blockInfos.begin(), blockInfos.end(), [](const BlockInfo& a, const BlockInfo& b) {
		return a.minDistToHole > b.minDistToHole;
		});

	// 自分の座標
	int selfX = static_cast<int>(model_->transform_.translate.x / kBlockSize);
	int selfY = static_cast<int>(model_->transform_.translate.z / kBlockSize);

	// 予測経路で到達可能な床ブロックを探索
	int bestX = selfX;
	int bestY = selfY;
	for (const auto& info : blockInfos) {
		// 予測移動（自分から目的地方向にGetMoveTimer()分進む）
		Vector2 startPos = { model_->transform_.translate.x, model_->transform_.translate.z };
		Vector2 goalPos = { info.x * kBlockSize + kBlockSize * 0.5f, info.y * kBlockSize + kBlockSize * 0.5f };
		Vector2 dir = (goalPos - startPos);
		if (dir.x == 0.0f && dir.y == 0.0f) continue;
		dir = dir / std::sqrt(dir.x * dir.x + dir.y * dir.y); // 正規化

		predictionLine_.Init();
		predictionLine_.Scan({ startPos.x, 1.0f, startPos.y }, dir, static_cast<int>(GetMoveTimer()), *wallMap_, kBlockSize);
		const auto& points = predictionLine_.GetLinePoints();

		// 最終到達点が目的地の床ブロック座標に近ければ到達可能とみなす
		if (!points.empty()) {
			const auto& last = points.back();
			int lastX = static_cast<int>(last.x / kBlockSize);
			int lastY = static_cast<int>(last.y / kBlockSize);
			if (lastX == info.x && lastY == info.y) {
				bestX = info.x;
				bestY = info.y;
				break;
			}
		}
	}
	mapCenterX_ = bestX * kBlockSize + kBlockSize * 0.5f;
	mapCenterY_ = bestY * kBlockSize + kBlockSize * 0.5f;
}

void Player::LifeEvaluation() {
	// 予測移動（directionTable_[i]方向にGetMoveTimer()分進む）
	Vector2 startPos = { model_->transform_.translate.x, model_->transform_.translate.z };
	Vector2 dir = moveDir_;

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
		lifeEvaluationValue_ = -100; // 死ぬ
	} else {
		lifeEvaluationValue_ = 5; // 盤外に落ちない
		// 中央に近いほど高評価
		float distToCenter = std::sqrt(
			(lastPos.x - mapCenterX_) * (lastPos.x - mapCenterX_) +
			(lastPos.y - mapCenterY_) * (lastPos.y - mapCenterY_)
		);
		// 何マス離れているか
		int blockDistance = static_cast<int>(distToCenter / kBlockSize);
		// 1ブロック以内なら8点、1マス離れるごとに-1点
		lifeEvaluationValue_ += 8 - blockDistance;
	}
}

void Player::AttackEvaluation() {
	// プレイヤー座標（事前にplayerPos_へセットされている前提）
	Vector2 playerPos = enemyPos_;
	// 予測移動（directionTable_[i]方向にGetMoveTimer()分進む）
	Vector2 startPos = { model_->transform_.translate.x, model_->transform_.translate.z };
	Vector2 dir = moveDir_.Normalize();

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
		(lastPos.x - mapCenterX_) * (lastPos.x - mapCenterX_) +
		(lastPos.y - mapCenterY_) * (lastPos.y - mapCenterY_)
	);
	float playerDistToCenter = std::sqrt(
		(playerPos.x - mapCenterX_) * (playerPos.x - mapCenterX_) +
		(playerPos.y - mapCenterY_) * (playerPos.y - mapCenterY_)
	);
	if (distToCenter < playerDistToCenter) {
		attackEvaluationValue_ += 3; // プレイヤーより中央に近い
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
		attackEvaluationValue_ += 2; // プレイヤーより広い場所
	}

}

void Player::UniqeEvaluation() {
	// 予測移動の最終到達点
	const auto& points = predictionLine_.GetLinePoints();
	if (!points.empty()) {
		Vector2 lastPos = points.back();

		// --- ここから合成法線方向のjumpDir計算 ---
		Vector2 totalNormal(0.0f, 0.0f);
		bool isCollided = false;
		Vector2 playerPos = { lastPos.x - 0.5f, lastPos.y - 0.5f };
		Vector2 playerCenter = playerPos + Vector2(0.5f, 0.5f);

		if (oldBuildMapChipIndex_ == nullptr) {
			assert(false && "oldBuildMapChipIndex_ is nullptr");
		}

		for (auto& mapchip : (*oldBuildMapChipIndex_)) {
			Vector2 mapChipPos = { static_cast<float>(mapchip.x) * kBlockSize - (kBlockSize * 0.5f), static_cast<float>(mapchip.y) * kBlockSize - (kBlockSize * 0.5f) };

			if (MapChipCollider::IsAABBCollision(playerPos, 1.0f, 1.0f, mapChipPos, kBlockSize, kBlockSize)) {
				Vector2 blockCenter = mapChipPos + Vector2(kBlockSize * 0.5f, kBlockSize * 0.5f);
				Vector2 jumpDir = (playerCenter - blockCenter).Normalize();
				totalNormal += jumpDir;
				isCollided = true;
			}
		}

		Vector2 jumpDir;
		if (isCollided) {
			if (totalNormal.Length() > 0.01f) {
				jumpDir = totalNormal.Normalize();
			} else {
				jumpDir = Vector2(0.0f, 1.0f);
			}
		} else {
			jumpDir = moveDir_.Normalize(); // 衝突していない場合は元の方向
		}
		// --- ここまで合成法線方向のjumpDir計算 ---

		float vx = jumpDir.x * 2.5f;
		float vz = jumpDir.y * 2.5f;
		float vy = 15.0f;
		float gravity = -9.81f;

		float t = (-2.0f * vy) / gravity;

		float jumpX = lastPos.x + vx * t;
		float jumpZ = lastPos.y + vz * t;

		int mapX = static_cast<int>(jumpX / kBlockSize);
		int mapY = static_cast<int>(jumpZ / kBlockSize);

		bool hasFloor = false;
		if (floorMap_ &&
			mapY >= 0 && mapY < static_cast<int>(floorMap_->size()) &&
			mapX >= 0 && mapX < static_cast<int>((*floorMap_)[mapY].size())) {
			hasFloor = ((*floorMap_)[mapY][mapX] != 0);
		}

		// 着地地点と中心の距離
		float distToCenter = std::sqrt(
			(jumpX - mapCenterX_) * (jumpX - mapCenterX_) +
			(jumpZ - mapCenterY_) * (jumpZ - mapCenterY_)
		);
		int blockDistance = static_cast<int>(distToCenter / kBlockSize);

		// 床があれば中心に近いほど高得点
		if (hasFloor) {
			uniqeEvaluationValue_ = 10 + (8 - blockDistance); // 1ブロック以内なら最大18点、遠いほど減点
		} else {
			uniqeEvaluationValue_ = -100;
		}
	}
}