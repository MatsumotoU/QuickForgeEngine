#include "GameScene.h"
#include "TitleScene.h"
#include "StageSelectScene.h"

#include "Class/GameScene/Collition/MapChipCollider.h"
#include "Class/GameScene/Collition/MapReflection.h"

#include "Utility/MyEasing.h"

GameScene::GameScene(EngineCore* engineCore, nlohmann::json* data) {
	sceneData_ = data;
	engineCore_ = engineCore;
	input_ = engineCore_->GetInputManager();

	camera_.transform_.translate = { 4.0f,19.0f,-4.8f };
	camera_.transform_.rotate.x = 1.14f;

	engineCore_->GetGraphRenderer()->SetCamera(&camera_);
	engineCore_->GetLoopStopper()->AddNonStoppingFunc(std::bind(&GameScene::CameraUpdate, this));

	sceneID_ = 1;

	buildMapChipIndex_.clear();

	nextScene_ = nullptr;

	// ステージ名の取得
	if (sceneData_->contains("stage")) {
		int stage = sceneData_->at("stage").get<int>();
		if (stage + 1 < 1) {
			stage = 1;
		} else if (stage + 1 > 8) {
			stage = 8;
		}
		stageName_ = "Stage" + std::to_string(stage+1);

	} else {

		stageName_ = "Stage1";
	}

	std::string wallFilePath = "Resources/Map/" + stageName_ + "_wall.csv";
	std::string floorFilePath = "Resources/Map/" + stageName_ + "_floor.csv";
	wallMap_ = MapChipLoader::Load(wallFilePath);
	floorMap_ = MapChipLoader::Load(floorFilePath);

	timer_ = 0.0f;
	cameraShakeTimer_ = 0.0f;

	bgmHandle_ = engineCore_->LoadSoundData("Resources/Sound/BGM/","GameSceneBGM.mp3");
	engineCore_->GetAudioPlayer()->PlayAudio(bgmHandle_, "GameSceneBGM.mp3", true);
}

GameScene::~GameScene() {
	engineCore_->GetPostprocess()->grayScaleOffset_ = 0.0f;
	engineCore_->GetGraphRenderer()->DeleteCamera(&camera_);
	engineCore_->GetAudioPlayer()->StopAudio("GameSceneBGM.mp3");
}

void GameScene::Initialize() {
	timer_ = 0.0f;

	camera_.Initialize(engineCore_->GetWinApp());
	isRequestedExit_ = false;
	collisionManager_.Initalize();

	floorChip_.Initialize(engineCore_, &camera_);
	wallChip_.Initialize(engineCore_, &camera_);
	floorChip_.SetMapPosition({ 0.0f,0.0f,0.0f });
	wallChip_.SetMapPosition({ 0.0f,1.0f,0.0f });
	predictionLine_.Init();
	landingPoint_.Init();

	player_.Initialize(engineCore_, &camera_);
	player_.GetTransform().translate.x += 2.0f;
	player_.GetTransform().translate.z += 4.0f;
	player_.SetOldBuildMapChipIndex(&oldBuildMapChipIndex_);

	enemy_.Initialize(engineCore_, &camera_);
	enemy_.GetTransform().translate.x += 6.0f;
	enemy_.GetTransform().translate.z += 4.0f;

	// AIレベルの取得
	std::string aiFilePath = "Resources/Map/" + stageName_ + "_ai.csv";
	aiLeveler_.LoadAIConfig(aiFilePath);
	enemy_.SetAiLevel(aiLeveler_.GetAILevel());
	enemy_.SetAiWeight(aiLeveler_.GetLifeWeight(), aiLeveler_.GetAttackWeight(), aiLeveler_.GetUniqueWeight());

	mainMenu_.Initialize(engineCore_, &camera_);
	resultUI_.Initialize(engineCore_, &camera_);
	turnText_.Initialize(engineCore_, &camera_);

	isPlayerTurn_ = true;
	isEndGame_ = false;
	isOpenMenu_ = false;

	endGameTimer_ = 0.0f;

	skyDome_.Initialize(engineCore_, &camera_);

	particleManager_.Initialize(engineCore_, &camera_);
	frameCount_ = 0;

	stageNumber_.Initialize(engineCore_, &camera_);
	if (stageName_ == "Stage1") {
		stageNumber_.SetNumber(1);
	} else if (stageName_ == "Stage2") {
		stageNumber_.SetNumber(2);
	} else if (stageName_ == "Stage3") {
		stageNumber_.SetNumber(3);
	} else if (stageName_ == "Stage4") {
		stageNumber_.SetNumber(4);
	} else if (stageName_ == "Stage5") {
		stageNumber_.SetNumber(5);
	} else if (stageName_ == "Stage6") {
		stageNumber_.SetNumber(6);
	} else if (stageName_ == "Stage7") {
		stageNumber_.SetNumber(7);
	} else if (stageName_ == "Stage8") {
		stageNumber_.SetNumber(8);
	}

	stageTextModel_ = std::make_unique<Model>(engineCore_, &camera_);
	stageTextModel_->LoadModel("Resources/Model/UI", "StageText.obj", COORDINATESYSTEM_HAND_RIGHT);
	stageTextModel_->transform_.translate = { -1.4f,3.5f,3.3f };
	stageTextModel_->transform_.rotate = { -1.2f,-3.6f,0.0f };
	stageTextModel_->transform_.scale = { 0.5f,0.5f,1.0f };
}

void GameScene::Update() {
	stageTextModel_->Update();
	frameCount_++;
	particleManager_.Update();
	skyDome_.Update();
	stageNumber_.Update();

	timer_ += engineCore_->GetDeltaTime();
	resultUI_.Update();
	turnText_.Update();
	turnText_.SetIsHidden(isEndGame_);

	if (cameraShakeTimer_ > 0.0f) {
		camera_.transform_.translate.x = 4.0f + sinf(cameraShakeTimer_ * 10.0f) * (cameraShakeTimer_ * 0.5f);
		cameraShakeTimer_ -= engineCore_->GetDeltaTime();
	} else {
		cameraShakeTimer_ = 0.0f;
		camera_.transform_.translate.x = 4.0f;
	}

	camera_.Update();

	// ゲーム終了チェック
	if (isEndGame_) {
		if (endGameTimer_ > 0.0f) {
			endGameTimer_ -= engineCore_->GetDeltaTime();
			// 死亡演出スキップ
			if (engineCore_->GetInputManager()->keyboard_.GetTrigger(DIK_SPACE) || engineCore_->GetXInputController()->GetTriggerButton(XINPUT_GAMEPAD_A, 0)) {
				endGameTimer_ = 0.0f;
			}	
			CheckEndGame();

		} else {
			MyEasing::SimpleEaseIn(&camera_.transform_.translate.x, 4.0f, 0.1f);
			MyEasing::SimpleEaseIn(&camera_.transform_.translate.z, -4.8f, 0.1f);
			MyEasing::SimpleEaseIn(&camera_.transform_.rotate.x, 0.0f, 0.1f);
			MyEasing::SimpleEaseIn(&camera_.transform_.rotate.y, 3.14f, 0.1f);

			if (engineCore_->GetInputManager()->keyboard_.GetTrigger(DIK_SPACE) || engineCore_->GetXInputController()->GetTriggerButton(XINPUT_GAMEPAD_A, 0)) {
				if (resultUI_.GetSelectedTop()) {
					if (player_.GetIsAlive()) {
						std::string nextStageName;
						if (stageName_ == "Stage1") {
							nextStageName = "Stage2";
						} else if (stageName_ == "Stage2") {
							nextStageName = "Stage3";
						} else if (stageName_ == "Stage3") {
							nextStageName = "Stage4";
						} else if (stageName_ == "Stage4") {
							nextStageName = "Stage5";
						} else if (stageName_ == "Stage5") {
							nextStageName = "Stage6";
						} else if (stageName_ == "Stage6") {
							nextStageName = "Stage7";
						} else if (stageName_ == "Stage7") {
							nextStageName = "Stage8";
						} else {
							nextStageName = "Stage1";
						}
						stageName_ = nextStageName;
					} 
					ResetGame(stageName_);
					
				} else {
					isRequestedExit_ = true;
				}
			}
		}

		player_.SetAlpha(1.0f);
		enemy_.SetAlpha(1.0f);
		return;
	}

	// メニューの開閉
	if (input_->keyboard_.GetTrigger(DIK_ESCAPE) || engineCore_->GetXInputController()->GetTriggerButton(XINPUT_GAMEPAD_START, 0)) {
		isOpenMenu_ = !isOpenMenu_;
	}

	// メニューとゲームの更新切り替え
	if (!isOpenMenu_) {
		MainGameUpdate();
	} else {
		MenuUpdate();
	}
}

void GameScene::Draw() {
	stageTextModel_->Draw();
	particleManager_.Draw();
	skyDome_.Draw();

	floorChip_.Draw();
	wallChip_.Draw();

	player_.Draw();
	enemy_.Draw();

	if (isOpenMenu_) {
		mainMenu_.Draw();
	}

	
	if (isEndGame_) {
		resultUI_.Draw();
	}
	turnText_.Draw();

	stageNumber_.Draw();
	predictionLine_.Draw(engineCore_);
	//landingPoint_.Draw(engineCore_);
}


IScene* GameScene::GetNextScene() {
	if (nextScene_ == nullptr) {
		return new StageSelectScene(engineCore_,sceneData_);
	}
	return nextScene_;
}
void GameScene::ChangeNextScene(IScene* nextScene) {
	if (nextScene_ != nullptr) {
		delete nextScene_;
	}
	nextScene_ = nextScene;
}

void GameScene::MainGameUpdate() {
	if (engineCore_->GetXInputController()->GetRightStick(0).Length() != 0.0f) {
		Vector2 rightStick = engineCore_->GetXInputController()->GetRightStick(0).Normalize();
		MyEasing::SimpleEaseIn(&camera_.transform_.rotate.x, 1.14f + -rightStick.y * 0.1f, 0.1f);
		MyEasing::SimpleEaseIn(&camera_.transform_.rotate.y, 0.0f + rightStick.x * 0.1f, 0.1f);

	} else {
		MyEasing::SimpleEaseIn(&camera_.transform_.rotate.x, 1.14f, 0.1f);
		MyEasing::SimpleEaseIn(&camera_.transform_.rotate.y, 0.0f, 0.1f);
	}

	wallChip_.SetMap(wallMap_);
	floorChip_.SetMap(floorMap_);

	floorChip_.Update();
	wallChip_.Update();

	
	enemy_.SetMap(&floorMap_, &wallMap_);
	enemy_.SetPlayerPos({ player_.GetTransform().translate.x,player_.GetTransform().translate.z });
	enemy_.Update();

	player_.SetMap(&floorMap_, &wallMap_);
	player_.SetEnemyPos({ enemy_.GetTransform().translate.x,enemy_.GetTransform().translate.z });
	player_.Update();

	// IngameUpdate
	MapChipUpdate(player_);
	MapChipUpdate(enemy_);
	BuildingMapChipUpdate(player_);
	BuildingMapChipUpdate(enemy_);
	JumpingUpdate(player_);
	JumpingUpdate(enemy_);
	GroundingUpdate(player_);
	GroundingUpdate(enemy_);

	AliveCheck(player_);
	AliveCheck(enemy_);

	// ターン交換
	if (isPlayerTurn_) {
		PredictionLineUpdate(player_);
		if (player_.GetIsEndTurn()) {
			oldBuildMapChipIndex_.clear();
			isPlayerTurn_ = false;
			enemy_.GetIsCanMove() = true;

			turnText_.ChangeTurn(isPlayerTurn_);
		}

		enemy_.SetAlpha(0.2f + sinf(timer_) * 0.1f);
		player_.SetAlpha(1.0f);

	} else {
		PredictionLineUpdate(enemy_);
		if (enemy_.GetIsEndTurn()) {
			oldBuildMapChipIndex_.clear();
			isPlayerTurn_ = true;
			player_.GetIsCanMove() = true;
			
			turnText_.ChangeTurn(isPlayerTurn_);
		}

		enemy_.SetAlpha(1.0f);
		player_.SetAlpha(0.2f + sinf(timer_) * 0.1f);
	}
}
void GameScene::MenuUpdate() {
	MyEasing::SimpleEaseIn(&camera_.transform_.rotate.x, 0.0f, 0.1f);
	MyEasing::SimpleEaseIn(&camera_.transform_.rotate.y, 3.14f, 0.1f);

	mainMenu_.Update();

	if (engineCore_->GetInputManager()->keyboard_.GetTrigger(DIK_SPACE) || engineCore_->GetXInputController()->GetTriggerButton(XINPUT_GAMEPAD_A, 0)) {
		if (mainMenu_.GetMenuSelect() == MenuSelect::ReturnSelect) {
			isRequestedExit_ = true;

		} else if (mainMenu_.GetMenuSelect() == MenuSelect::ResetGame) {
			ResetGame(stageName_);
			isOpenMenu_ = false;
		}
	}
}

void GameScene::ResetGame(const std::string& stageName) {
	wallMap_ = MapChipLoader::Load("Resources/Map/" + stageName + "_wall.csv");
	floorMap_ = MapChipLoader::Load("Resources/Map/" + stageName + "_floor.csv");
	std::string aiFilePath = "Resources/Map/" + stageName_ + "_ai.csv";
	aiLeveler_.LoadAIConfig(aiFilePath);
	enemy_.SetAiLevel(aiLeveler_.GetAILevel());
	enemy_.SetAiWeight(aiLeveler_.GetLifeWeight(), aiLeveler_.GetAttackWeight(), aiLeveler_.GetUniqueWeight());
	buildMapChipIndex_.clear();
	oldBuildMapChipIndex_.clear();
	player_.RestParameter();
	enemy_.RestParameter();
	player_.GetTransform().translate = { 2.0f,0.0f,4.0f };
	player_.SetAlive(true);
	player_.SetAlpha(1.0f);
	player_.ResetForce();
	enemy_.GetTransform().translate = { 6.0f,0.0f,4.0f };
	enemy_.SetAlive(true);
	enemy_.SetAlpha(1.0f);
	enemy_.ResetForce();
	player_.GetIsCanMove() = true;
	enemy_.GetIsCanMove() = false;
	isPlayerTurn_ = true;
	isEndGame_ = false;
	camera_.transform_.translate = { 4.0f,19.0f,-4.8f };
	camera_.transform_.rotate.x = 1.14f;
	engineCore_->GetPostprocess()->grayScaleOffset_ = 0.0f;
	turnText_.ChangeTurn(isPlayerTurn_);

	if (stageName_ == "Stage1") {
		stageNumber_.SetNumber(1);
	} else if (stageName_ == "Stage2") {
		stageNumber_.SetNumber(2);
	} else if (stageName_ == "Stage3") {
		stageNumber_.SetNumber(3);
	} else if (stageName_ == "Stage4") {
		stageNumber_.SetNumber(4);
	} else if (stageName_ == "Stage5") {
		stageNumber_.SetNumber(5);
	} else if (stageName_ == "Stage6") {
		stageNumber_.SetNumber(6);
	} else if (stageName_ == "Stage7") {
		stageNumber_.SetNumber(7);
	} else if (stageName_ == "Stage8") {
		stageNumber_.SetNumber(8);
	}
}
void GameScene::CameraUpdate() {
}
void GameScene::PredictionLineUpdate(GamePlayer& gamePlayer) {
	predictionLine_.Init();
	if (gamePlayer.GetIsCanMove()) {
		predictionLine_.Scan(
			gamePlayer.GetTransform().translate,
			gamePlayer.GetMoveDir(),
			static_cast<int>(gamePlayer.GetMoveTimer()), // 1秒間に進むマス数
			wallMap_,
			kBlockSize
		);

		// ブロック建築予測描画
		floorChip_.ResetChipColor();
		const auto& linePoints = predictionLine_.GetLinePoints();
		if (!linePoints.empty()) {
			// まず全て塗る
			std::vector<Vector2> buildPoints;
			for (const auto& point : linePoints) {
				IntVector2 mapChipPos;
				mapChipPos.x = static_cast<int>(std::round(point.x));
				mapChipPos.y = static_cast<int>(std::round(point.y));
				
				buildPoints.push_back({ static_cast<float>(mapChipPos.x), static_cast<float>(mapChipPos.y) });
			}
			Vector2 lastPoint = buildPoints.back();
			buildPoints.pop_back();
			for (const auto& buildPoint : buildPoints) {
				floorChip_.SetChipColor(static_cast<int>(buildPoint.x), static_cast<int>(buildPoint.y), { 0.0f, 1.0f, 0.0f, 1.0f });
			}

			// 着地点の予測表示
			/*landingPoint_.Init();
			landingPoint_.Scan({ lastPoint.x ,1.0f,lastPoint.y }, oldBuildMapChipIndex_, floorMap_, 1.0f);*/
		}
	}
}
void GameScene::MapChipUpdate(GamePlayer& gamePlayer) {
	if (gamePlayer.GetIsMoving() && !gamePlayer.GetIsJumping()) {
		float minDistance = std::numeric_limits<float>::max();
		int nearestX = -1;
		int nearestY = -1;
		Vector2 nearestMapChipPos;
		bool isCollided = false;

		if (frameCount_ % 7 == 0) {
			Vector3 emmitPos = gamePlayer.GetWorldPosition();
			emmitPos.y += 0.5f;
			particleManager_.EmitBomb(emmitPos, gamePlayer.GetColor(), 3, 1.0f, 0.98f, 50);
		}

		Vector2 playerPos = { gamePlayer.GetTransform().translate.x - 0.5f, gamePlayer.GetTransform().translate.z - 0.5f };
		Vector2 playerCenter = playerPos + Vector2(0.5f, 0.5f); // プレイヤーの中央座標

		for (int y = 0; y < wallMap_.size(); y++) {
			for (int x = 0; x < wallMap_[y].size(); x++) {
				if (wallMap_[y][x] != 0) {
					Vector2 mapChipPos = { static_cast<float>(x) * kBlockSize - (kBlockSize * 0.5f), static_cast<float>(y) * kBlockSize - (kBlockSize * 0.5f) };
					Vector2 blockCenter = mapChipPos + Vector2(kBlockSize * 0.5f, kBlockSize * 0.5f);
					float distance = Vector2::Distance(playerCenter, blockCenter);
					if (MapChipCollider::IsAABBCollision(playerPos, 1.0f, 1.0f, mapChipPos, kBlockSize, kBlockSize)) {
						if (distance < minDistance) {
							minDistance = distance;
							nearestX = x;
							nearestY = y;
							nearestMapChipPos = mapChipPos;
							isCollided = true;
						}
					}
				}
			}
		}

		// 一番近いブロックでのみ反射
		static bool reflectedThisFrame = false; // 1フレームで1回だけ反射
		if (isCollided && !reflectedThisFrame) {
			MapReflection::ReflectPlayer(gamePlayer.GetMoveDir(), playerPos, 1.0f, 1.0f, nearestMapChipPos, kBlockSize, kBlockSize);
			reflectedThisFrame = true;

			// めり込み防止: プレイヤー座標をブロック外へ押し戻す
			Vector2 reflectDir = (playerCenter - (nearestMapChipPos + Vector2(kBlockSize * 0.5f, kBlockSize * 0.5f))).Normalize();
			gamePlayer.GetTransform().translate.x += reflectDir.x * 0.1f;
			gamePlayer.GetTransform().translate.z += reflectDir.y * 0.1f;

			if (gamePlayer.GetIsMoving()) {
				particleManager_.EmitBomb(
					{ nearestMapChipPos.x + 0.5f,0.5f,nearestMapChipPos.y + 0.5f },
					{ 1.0f,0.5f,0.0f,1.0f }, 5, 1.0f, 0.98f, 100);

				if (wallMap_[nearestY][nearestX] == 3) {
					wallMap_[nearestY][nearestX] = 1;
				} else {
					wallMap_[nearestY][nearestX] = 0;
				}

			}
		} else {
			reflectedThisFrame = false;
		}
	}
}
void GameScene::BuildingMapChipUpdate(GamePlayer& gamePlayer) {
	// 建築場所保存
	if (gamePlayer.GetIsMoving()) {
		IntVector2 mapChipPos;
		mapChipPos.x = static_cast<int>(std::round(gamePlayer.GetTransform().translate.x));
		mapChipPos.y = static_cast<int>(std::round(gamePlayer.GetTransform().translate.z));

		// 範囲外チェック
		if (mapChipPos.x >= 0 && mapChipPos.x < static_cast<int>(wallMap_[0].size()) &&
			mapChipPos.y >= 0 && mapChipPos.y < static_cast<int>(wallMap_.size())) {

			// 重複排除
			if (buildMapChipIndex_.empty() ||
				(buildMapChipIndex_.back().x != mapChipPos.x ||
					buildMapChipIndex_.back().y != mapChipPos.y)) {
				// 既に登録済みなら追加しない
				if (std::find(buildMapChipIndex_.begin(), buildMapChipIndex_.end(), mapChipPos) == buildMapChipIndex_.end()) {
					buildMapChipIndex_.push_back(mapChipPos);
				}
			}

			// 一個前のブロックを赤く塗る
			if (buildMapChipIndex_.size() >= 2) {
				const IntVector2& prevPos = buildMapChipIndex_[buildMapChipIndex_.size() - 2];
				floorChip_.SetChipColor(prevPos.x, prevPos.y, { 1.0f, 0.0f, 0.0f, 1.0f });
			}
		}
	}
	// 建築
	if (gamePlayer.GetIsBuilding()) {
		// 一個前の座標は削除
		if (!buildMapChipIndex_.empty()) {
			buildMapChipIndex_.pop_back();
		}

		oldBuildMapChipIndex_.clear();
		for (const auto& index : buildMapChipIndex_) {
			// 範囲外チェック
			if (index.x >= 0 && index.x < static_cast<int>(wallMap_[0].size()) &&
				index.y >= 0 && index.y < static_cast<int>(wallMap_.size())) {
				oldBuildMapChipIndex_.push_back(index);

				if (wallMap_[index.y][index.x] == 0) {
					if (floorMap_[index.y][index.x] != 0) {
						wallMap_[index.y][index.x] = 4;
					}
				}
			}
		}
		gamePlayer.SetIsBuilding(false);
		floorChip_.ResetChipColor();

		if (cameraShakeTimer_ < 0.5f) {
			cameraShakeTimer_ += 0.5f;
		}

		buildMapChipIndex_.clear();
	}
}
void GameScene::JumpingUpdate(GamePlayer& gamePlayer) {
	if (!gamePlayer.GetIsMoving()) {
		Vector2 totalNormal(0.0f, 0.0f);
		bool isCollided = false;
		Vector2 playerPos = { gamePlayer.GetTransform().translate.x - 0.5f, gamePlayer.GetTransform().translate.z - 0.5f };
		Vector2 playerCenter = playerPos + Vector2(0.5f, 0.5f);

		for (auto& mapchip : oldBuildMapChipIndex_) {
			Vector2 mapChipPos = { static_cast<float>(mapchip.x) * kBlockSize - (kBlockSize * 0.5f), static_cast<float>(mapchip.y) * kBlockSize - (kBlockSize * 0.5f) };

			if (MapChipCollider::IsAABBCollision(playerPos, 1.0f, 1.0f, mapChipPos, kBlockSize, kBlockSize)) {
				Vector2 blockCenter = mapChipPos + Vector2(kBlockSize * 0.5f, kBlockSize * 0.5f);
				Vector2 jumpDir = (playerCenter - blockCenter).Normalize();
				totalNormal += jumpDir;
				isCollided = true;
			}
		}

		// 合成方向でジャンプ（ゼロベクトルの場合は上方向にジャンプ）
		if (isCollided) {
			Vector2 jumpDir;
			if (totalNormal.Length() > 0.01f) { // 0.0f だと不安定なので閾値を設ける
				jumpDir = totalNormal.Normalize();
			} else {
				jumpDir = Vector2(0.0f, 1.0f); // デフォルトで上方向
			}
			gamePlayer.Jamp(jumpDir);
		}
	}
}
void GameScene::GroundingUpdate(GamePlayer& gamePlayer) {
	if (gamePlayer.GetIsGrounded()) {
		IntVector2 mapChipPos;
		mapChipPos.x = static_cast<int>(std::round(gamePlayer.GetTransform().translate.x));
		mapChipPos.y = static_cast<int>(std::round(gamePlayer.GetTransform().translate.z));

		for (int dy = -1; dy <= 1; ++dy) {
			for (int dx = -1; dx <= 1; ++dx) {
				int nx = mapChipPos.x + dx;
				int ny = mapChipPos.y + dy;
				if (nx >= 0 && nx < static_cast<int>(wallMap_[0].size()) &&
					ny >= 0 && ny < static_cast<int>(wallMap_.size())) {
					wallMap_[ny][nx] = 0;
					particleManager_.EmitBomb(
						{ static_cast<float>(nx),1.0f,static_cast<float>(ny) },
						{ 1.0f,0.5f,0.0f,1.0f }, 5, 1.0f, 0.98f, 100);
				}
			}
		}
		gamePlayer.SetGrounded(false);

		// 床があれば揺らす
		if (mapChipPos.x >= 0 && mapChipPos.x < static_cast<int>(wallMap_[0].size()) &&
			mapChipPos.y >= 0 && mapChipPos.y < static_cast<int>(wallMap_.size())) {
			if (cameraShakeTimer_ < 0.5f) {
				cameraShakeTimer_ += 0.5f;
				particleManager_.EmitBomb(
					{ static_cast<float>(mapChipPos.x),1.0f,static_cast<float>(mapChipPos.y)},
					{ 1.0f,1.0f,1.0f,1.0f }, 10, 1.0f, 0.98f, 50);
			}
		}
	}
}
void GameScene::AliveCheck(GamePlayer& gamePlayer) {
	if (gamePlayer.GetIsJumping()) {
		return;
	}

	bool isCollided = false;

	Vector2 playerPos = { gamePlayer.GetTransform().translate.x - 0.5f, gamePlayer.GetTransform().translate.z - 0.5f };
	Vector2 playerCenter = playerPos + Vector2(0.5f, 0.5f);

	for (int y = 0; y < floorMap_.size(); y++) {
		for (int x = 0; x < floorMap_[y].size(); x++) {
			if (floorMap_[y][x] != 0) {
				Vector2 mapChipPos = { static_cast<float>(x) * kBlockSize - (kBlockSize * 0.5f), static_cast<float>(y) * kBlockSize - (kBlockSize * 0.5f) };
				if (MapChipCollider::IsAABBCollision(playerPos, 1.0f, 1.0f, mapChipPos, kBlockSize, kBlockSize)) {
					isCollided = true;
				}
			}
		}
	}

	if (isCollided == false) {
		isEndGame_ = true;
		gamePlayer.SetAlive(false);
		endGameTimer_ = 1.5f;
	}
}
void GameScene::CheckEndGame() {
	// ゲーム終了処理
	if (isEndGame_) {
		if (!enemy_.GetIsAlive()) {
			enemy_.DeathAnimation();

			MyEasing::SimpleEaseIn(&camera_.transform_.translate.x, enemy_.GetTransform().translate.x, 0.1f);
			MyEasing::SimpleEaseIn(&camera_.transform_.translate.z, -7.8f + enemy_.GetTransform().translate.z, 0.1f);

			resultUI_.isCleard_ = true;

		} else if (!player_.GetIsAlive()) {
			player_.DeathAnimation();

			MyEasing::SimpleEaseIn(&camera_.transform_.translate.x, player_.GetTransform().translate.x, 0.1f);
			MyEasing::SimpleEaseIn(&camera_.transform_.translate.z, -7.8f + player_.GetTransform().translate.z, 0.1f);

			MyEasing::SimpleEaseIn(&engineCore_->GetPostprocess()->grayScaleOffset_, 1.0f, 0.01f);

			resultUI_.isCleard_ = false;
		}

		return;
	}
}
