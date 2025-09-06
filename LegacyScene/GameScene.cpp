#include "GameScene.h"
#include "TitleScene.h"

#include "Class/GameScene/Collition/MapChipCollider.h"
#include "Class/GameScene/Collition/MapReflection.h"


#include "Utility/MyEasing.h"

GameScene::GameScene(EngineCore* engineCore) :debugCamera_(engineCore) {
	engineCore_ = engineCore;
	input_ = engineCore_->GetInputManager();

	camera_.transform_.translate = { 4.0f,19.0f,-4.8f };
	camera_.transform_.rotate.x = 1.14f;

#ifdef _DEBUG
	isActiveDebugCamera_ = false;
	debugCamera_.Initialize(engineCore_);
	debugCamera_.camera_.transform_.translate.z = -20.0f;
#endif // _DEBUG

	engineCore_->GetGraphRenderer()->SetCamera(&camera_);
	engineCore_->GetLoopStopper()->AddNonStoppingFunc(std::bind(&GameScene::CameraUpdate, this));

	sceneID_ = 1;

	wallMap_ = MapChipLoader::Load("Resources/Map/Stage1_wall.csv");
	floorMap_ = MapChipLoader::Load("Resources/Map/Stage1_floor.csv");

	buildMapChipIndex_.clear();

	nextScene_ = nullptr;

	stageName_ = "Stage1";

}

GameScene::~GameScene() {
	engineCore_->GetGraphRenderer()->DeleteCamera(&camera_);
}

void GameScene::Initialize() {
	camera_.Initialize(engineCore_->GetWinApp());
	isRequestedExit_ = false;
	collisionManager_.Initalize();

	floorChip_.Initialize(engineCore_, &camera_);
	wallChip_.Initialize(engineCore_, &camera_);
	floorChip_.SetMapPosition({ 0.0f,0.0f,0.0f });
	wallChip_.SetMapPosition({ 0.0f,1.0f,0.0f });
	predictionLine_.Init();

	player_.Initialize(engineCore_, &camera_);
	player_.GetTransform().translate.x += 2.0f;
	player_.GetTransform().translate.z += 4.0f;

	enemy_.Initialize(engineCore_, &camera_);
	enemy_.GetTransform().translate.x += 6.0f;
	enemy_.GetTransform().translate.z += 4.0f;

	mainMenu_.Initialize(engineCore_, &camera_);

	isPlayerTurn_ = true;
	isEndGame_ = false;
	isOpenMenu_ = false;
}

void GameScene::Update() {
	camera_.Update();
#ifdef _DEBUG
	if (input_->keyboard_.GetTrigger(DIK_R)) {
		ResetGame(stageName_);
	}

	if (input_->keyboard_.GetTrigger(DIK_P)) {
		isActiveDebugCamera_ = !isActiveDebugCamera_;
	}
	CameraUpdate();
#endif // _DEBUG

	// ゲーム終了チェック
	CheckEndGame();
	if (isEndGame_) {
		return;
	}

	// メニューの開閉
	if (input_->keyboard_.GetTrigger(DIK_ESCAPE) || engineCore_->GetXInputController()->GetTriggerButton(XINPUT_GAMEPAD_START,0)) {
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
#ifdef _DEBUG
	debugCamera_.DrawImGui();
#endif // _DEBUG

	floorChip_.Draw();
	wallChip_.Draw();

	player_.Draw();
	enemy_.Draw();

	mainMenu_.Draw();

	predictionLine_.Draw(engineCore_);
}


IScene* GameScene::GetNextScene() {
	if (nextScene_ == nullptr) {
		return new TitleScene(engineCore_);
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
	MyEasing::SimpleEaseIn(&camera_.transform_.rotate.x, 1.14f, 0.1f);
	MyEasing::SimpleEaseIn(&camera_.transform_.rotate.y, 0.0f, 0.1f);

	wallChip_.SetMap(wallMap_);
	floorChip_.SetMap(floorMap_);

	floorChip_.Update();
	wallChip_.Update();

	player_.SetMap(&floorMap_, &wallMap_);
	enemy_.SetMap(&floorMap_, &wallMap_);
	enemy_.SetPlayerPos({ player_.GetTransform().translate.x,player_.GetTransform().translate.z });
	player_.Update();
	enemy_.Update();

	// IngameUpdate
	MapChipUpdate(player_);
	BuildingMapChipUpdate(player_);
	MapChipUpdate(enemy_);
	BuildingMapChipUpdate(enemy_);
	JumpingUpdate(player_);
	GroundingUpdate(player_);
	JumpingUpdate(enemy_);
	GroundingUpdate(enemy_);

	AliveCheck(player_);
	AliveCheck(enemy_);

	// ターン交換
	if (isPlayerTurn_) {
		PredictionLineUpdate(player_);
		if (player_.GetIsEndTurn()) {
			isPlayerTurn_ = false;
			enemy_.GetIsCanMove() = true;
			player_.SetAlpha(0.5f);
			enemy_.SetAlpha(1.0f);
		}
	} else {
		PredictionLineUpdate(enemy_);
		if (enemy_.GetIsEndTurn()) {
			isPlayerTurn_ = true;
			player_.GetIsCanMove() = true;
			player_.SetAlpha(1.0f);
			enemy_.SetAlpha(0.5f);
		}
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
	buildMapChipIndex_.clear();
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
}
void GameScene::CameraUpdate() {
#ifdef _DEBUG
	if (isActiveDebugCamera_) {
		debugCamera_.Update();
		camera_ = debugCamera_.camera_;
	}
#endif // _DEBUG
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
	}
}
void GameScene::MapChipUpdate(GamePlayer& gamePlayer) {
	// マップの当たり判定（最も近いブロックのみ反射）
	float minDistance = std::numeric_limits<float>::max();
	int nearestX = -1;
	int nearestY = -1;
	Vector2 nearestMapChipPos;
	bool isCollided = false;

	Vector2 playerPos = { gamePlayer.GetTransform().translate.x - 0.5f, gamePlayer.GetTransform().translate.z - 0.5f };
	Vector2 playerCenter = playerPos + Vector2(0.5f, 0.5f); // プレイヤーの中央座標

	for (int y = 0; y < wallMap_.size(); y++) {
		for (int x = 0; x < wallMap_[y].size(); x++) {
			if (wallMap_[y][x] != 0) {
				Vector2 mapChipPos = { static_cast<float>(x) * kBlockSize - (kBlockSize * 0.5f), static_cast<float>(y) * kBlockSize - (kBlockSize * 0.5f) };
				Vector2 blockCenter = mapChipPos + Vector2(kBlockSize * 0.5f, kBlockSize * 0.5f); // ブロックの中央座標
				float distance = Vector2::Distance(playerCenter, blockCenter); // 中央同士の距離で判定
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
	if (isCollided) {
		MapReflection::ReflectPlayer(gamePlayer.GetMoveDir(), playerPos, 1.0f, 1.0f, nearestMapChipPos, kBlockSize, kBlockSize);
		if (gamePlayer.GetIsMoving()) {
			wallMap_[nearestY][nearestX] = 0;
		}
	}
}
void GameScene::BuildingMapChipUpdate(GamePlayer& gamePlayer) {
	// 建築場所保存
	if (gamePlayer.GetIsMoving()) {
		IntVector2 mapChipPos;
		mapChipPos.x = static_cast<int>(std::round(gamePlayer.GetTransform().translate.x));
		mapChipPos.y = static_cast<int>(std::round(gamePlayer.GetTransform().translate.z));

		if (mapChipPos.x >= 0 && mapChipPos.x < static_cast<int>(wallMap_[0].size()) &&
			mapChipPos.y >= 0 && mapChipPos.y < static_cast<int>(wallMap_.size())) {

			if (buildMapChipIndex_.empty()) {
				buildMapChipIndex_.push_back(mapChipPos);
			} else if
				(buildMapChipIndex_.back().x != mapChipPos.x ||
					buildMapChipIndex_.back().y != mapChipPos.y) {
				buildMapChipIndex_.push_back(mapChipPos);
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
		for (const auto& index : buildMapChipIndex_) {
			if (wallMap_[index.y][index.x] == 0) {
				wallMap_[index.y][index.x] = 1;
			}
		}
		buildMapChipIndex_.clear();
		gamePlayer.SetIsBuilding(false);
		floorChip_.ResetChipColor();
	}
}
void GameScene::JumpingUpdate(GamePlayer& gamePlayer) {
	if (!gamePlayer.GetIsMoving()) {
		Vector2 totalNormal(0.0f, 0.0f);
		bool isCollided = false;
		Vector2 playerPos = { gamePlayer.GetTransform().translate.x - 0.5f, gamePlayer.GetTransform().translate.z - 0.5f };
		Vector2 playerCenter = playerPos + Vector2(0.5f, 0.5f);

		for (int y = 0; y < wallMap_.size(); y++) {
			for (int x = 0; x < wallMap_[y].size(); x++) {
				if (wallMap_[y][x] != 0) {
					Vector2 mapChipPos = { static_cast<float>(x) * kBlockSize - (kBlockSize * 0.5f), static_cast<float>(y) * kBlockSize - (kBlockSize * 0.5f) };
					if (MapChipCollider::IsAABBCollision(playerPos, 1.0f, 1.0f, mapChipPos, kBlockSize, kBlockSize)) {
						Vector2 blockCenter = mapChipPos + Vector2(kBlockSize * 0.5f, kBlockSize * 0.5f);
						Vector2 jumpDir = (playerCenter - blockCenter).Normalize();
						totalNormal += jumpDir;
						isCollided = true;
					}
				}
			}
		}

		// 合成方向でジャンプ（ゼロベクトルの場合は上方向にジャンプ）
		if (isCollided) {
			Vector2 jumpDir;
			if (totalNormal.Length() > 0.0f) {
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

		if (mapChipPos.x >= 0 && mapChipPos.x < static_cast<int>(wallMap_[0].size()) &&
			mapChipPos.y >= 0 && mapChipPos.y < static_cast<int>(wallMap_.size())) {

			// 周囲8マス＋中心の9マスを0にする
			for (int dy = -1; dy <= 1; ++dy) {
				for (int dx = -1; dx <= 1; ++dx) {
					int nx = mapChipPos.x + dx;
					int ny = mapChipPos.y + dy;
					if (nx >= 0 && nx < static_cast<int>(wallMap_[0].size()) &&
						ny >= 0 && ny < static_cast<int>(wallMap_.size())) {
						wallMap_[ny][nx] = 0;
					}
				}
			}
		}
		gamePlayer.SetGrounded(false);
	}
}
void GameScene::AliveCheck(GamePlayer& gamePlayer) {
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
	}
}
void GameScene::CheckEndGame() {
	// ゲーム終了処理
	if (isEndGame_) {
		if (!enemy_.GetIsAlive()) {
			MyEasing::SimpleEaseIn(&camera_.transform_.translate.x, enemy_.GetTransform().translate.x, 0.1f);
			MyEasing::SimpleEaseIn(&camera_.transform_.translate.z, -7.8f + enemy_.GetTransform().translate.z, 0.1f);
		} else if (!player_.GetIsAlive()) {
			MyEasing::SimpleEaseIn(&camera_.transform_.translate.x, player_.GetTransform().translate.x, 0.1f);
			MyEasing::SimpleEaseIn(&camera_.transform_.translate.z, -7.8f + player_.GetTransform().translate.z, 0.1f);

			MyEasing::SimpleEaseIn(&engineCore_->GetPostprocess()->grayScaleOffset_, 1.0f, 0.01f);

		}

		return;
	}
}
