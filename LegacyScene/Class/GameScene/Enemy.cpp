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
	delayTimer_ = 0.0f;

	selectedDirIndex_ = 0;
	maxSelect_ = 0;
	color_ = { 1.0f,0.0f,1.0f,1.0f };

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
			ScanMapCenter();
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
			velocity_.x = 0.0f;
			velocity_.z = 0.0f;

			isJumping_ = false;
			isGrounded_ = true;
		}
	}

	model_->SetColor({ 1.0f,0.0f,1.0f,alpha_ });
	model_->Update();
}

void Enemy::Draw() {
#ifdef _DEBUG
	ImGui::Begin("Enemy");

	int aiLevel = aiLevel_;
	ImGui::DragInt("AiLevel", &aiLevel);
	aiLevel_ = std::clamp(aiLevel, 1, 100);
	ImGui::DragFloat3("AIWeight", &aiWeight_.life, 0.1f, 0.0f, 1.0f);
	ImGui::Text("SelectIndex: %d MaxSelected: %d", selectedDirIndex_, maxSelect_);

	if (ImGui::BeginTable("SelectedDirectionTable", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
		ImGui::TableSetupColumn("Index");
		ImGui::TableSetupColumn("Direction");
		ImGui::TableSetupColumn("Evaluation");
		ImGui::TableHeadersRow();

		for (size_t i = 0; i < selectedDirIndexTable_.size(); ++i) {
			int idx = selectedDirIndexTable_[i];
			ImGui::TableNextRow();
			if (static_cast<int>(i) == selectedDirIndex_) {
				// 選択中の行の背景色を変更（例: 黄色）
				ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, IM_COL32(128, 0, 0, 255));
			}
			ImGui::TableNextColumn();
			ImGui::Text("%d", idx);
			ImGui::TableNextColumn();
			ImGui::Text("(%.2f, %.2f)", directionTable_[idx].x, directionTable_[idx].y);
			ImGui::TableNextColumn();
			ImGui::Text("%d", totalEvaluationValue_[idx]);
		}
		ImGui::EndTable();
	}

	Vector2 leftStick = engineCore_->GetXInputController()->GetLeftStick(0);
	ImGui::Text("%f,%f", leftStick.x, leftStick.y);

	ImGui::End();
#endif // _DEBUG
	
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

	float negativeKeepRate = 0.4938f * std::powf(1.0f - static_cast<float>(aiLevel_) / 100.0f, 2.0f);
	negativeKeepRate = std::clamp(negativeKeepRate, 0.0f, 0.4938f);

	uint32_t count = 0;
	for (int idx : dirTableIndexTable_) {
		if (count >= maxSelect) break;
		if (totalEvaluationValue_[idx] < 0) {
			if (static_cast<float>(rand()) / RAND_MAX < negativeKeepRate) {
				selectedDirIndexTable_.push_back(idx);
				++count;
			}
		} else {
			selectedDirIndexTable_.push_back(idx);
			++count;
		}
	}
	// もし選択肢が0なら、最低1つは残す
	if (selectedDirIndexTable_.empty() && !dirTableIndexTable_.empty()) {
		selectedDirIndexTable_.push_back(dirTableIndexTable_[0]);
	}
}

void Enemy::ShotRandomDir() {
	// ランダムに選択
	int randomIndex = rand() % selectedDirIndexTable_.size();
	selectedDirIndex_ = randomIndex;
	shotDir_ = directionTable_[selectedDirIndexTable_[randomIndex]];
}

void Enemy::ScanMapCenter() {
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

void Enemy::LifeEvaluation() {
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
			lifeEvaluationValue_[i] = -100; // 死ぬ
		} else {
			lifeEvaluationValue_[i] = 5; // 盤外に落ちない
			// 中央に近いほど高評価
			float distToCenter = std::sqrt(
				(lastPos.x - mapCenterX_) * (lastPos.x - mapCenterX_) +
				(lastPos.y - mapCenterY_) * (lastPos.y - mapCenterY_)
			);
			// 何マス離れているか
			int blockDistance = static_cast<int>(distToCenter / kBlockSize);
			// 1ブロック以内なら8点、1マス離れるごとに-1点
			lifeEvaluationValue_[i] += 8 - blockDistance;
		}
	}
}

void Enemy::AttackEvaluation() {
	// プレイヤー座標（事前にplayerPos_へセットされている前提）
	Vector2 playerPos = playerPos_;
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
			(lastPos.x - mapCenterX_) * (lastPos.x - mapCenterX_) +
			(lastPos.y - mapCenterY_) * (lastPos.y - mapCenterY_)
		);
		float playerDistToCenter = std::sqrt(
			(playerPos.x - mapCenterX_) * (playerPos.x - mapCenterX_) +
			(playerPos.y - mapCenterY_) * (playerPos.y - mapCenterY_)
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
	for (int i = 0; i < kCheckCount_; i++) {
		// 予測移動の最終到達点
		const auto& points = predictionLine_.GetLinePoints();
		if (!points.empty()) {
			Vector2 lastPos = points.back();
			Vector2 jumpDir = directionTable_[i].Normalize();

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
				uniqeEvaluationValue_[i] = 10 + (8 - blockDistance); // 1ブロック以内なら最大18点、遠いほど減点
			} else {
				uniqeEvaluationValue_[i] = -100;
			}
		}
	}
}
