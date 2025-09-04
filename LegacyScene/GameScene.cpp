#include "GameScene.h"
#include "TitleScene.h"

#include "Class/GameScene/Collition/MapChipCollider.h"
#include "Class/GameScene/Collition/MapReflection.h"

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

	map_ = MapChipLoader::Load("Resources/Map/Stage1.csv");

	buildMapChipIndex_.clear();
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

	player_.Initialize(engineCore_, &camera_);
	player_.GetTransform().translate.x += 4.0f;
	player_.GetTransform().translate.z += 4.0f;
}

void GameScene::Update() {
	camera_.Update();
#ifdef _DEBUG
	if (input_->keyboard_.GetTrigger(DIK_P)) {
		isActiveDebugCamera_ = !isActiveDebugCamera_;
	}
	CameraUpdate();
#endif // _DEBUG

	wallChip_.SetMap(map_);

	floorChip_.Update();
	wallChip_.Update();

	player_.Update();
	MapChipUpdate(player_);
	BuildingMapChipUpdate(player_);
	JumpingUpdate(player_);
	GroundingUpdate(player_);
}

void GameScene::Draw() {
#ifdef _DEBUG
	debugCamera_.DrawImGui();
#endif // _DEBUG

	floorChip_.Draw();
	wallChip_.Draw();

	player_.Draw();
}


IScene* GameScene::GetNextScene() {
	return new TitleScene(engineCore_);
}

void GameScene::CameraUpdate() {
#ifdef _DEBUG
	if (isActiveDebugCamera_) {
		debugCamera_.Update();
		camera_ = debugCamera_.camera_;
	}
#endif // _DEBUG
}

void GameScene::MapChipUpdate(GamePlayer& gamePlayer) {
	// マップの当たり判定（最も近いブロックのみ反射）
	float minDistance = std::numeric_limits<float>::max();
	int nearestX = -1;
	int nearestY = -1;
	Vector2 nearestMapChipPos;
	bool isCollided = false;

	Vector2 playerPos = { gamePlayer.GetTransform().translate.x - 0.5f, gamePlayer.GetTransform().translate.z - 0.5f };
	Vector2 playerCenter = playerPos + Vector2(0.5f, 0.5f);

	for (int y = 0; y < map_.size(); y++) {
		for (int x = 0; x < map_[y].size(); x++) {
			if (map_[y][x] != 0) {
				Vector2 mapChipPos = { static_cast<float>(x) * kBlockSize - (kBlockSize * 0.5f), static_cast<float>(y) * kBlockSize - (kBlockSize * 0.5f) };
				if (MapChipCollider::IsAABBCollision(playerPos, 1.0f, 1.0f, mapChipPos, kBlockSize, kBlockSize)) {
					Vector2 blockCenter = mapChipPos + Vector2(kBlockSize * 0.5f, kBlockSize * 0.5f);
					float distance = Vector2::Distance(playerCenter, blockCenter);
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
			map_[nearestY][nearestX] = 0;
		}
	}
}
void GameScene::BuildingMapChipUpdate(GamePlayer& gamePlayer) {
	// 建築場所保存
	if (gamePlayer.GetIsMoving()) {
		IntVector2 mapChipPos;
		mapChipPos.x = static_cast<int>(std::round(gamePlayer.GetTransform().translate.x));
		mapChipPos.y = static_cast<int>(std::round(gamePlayer.GetTransform().translate.z));

		if (mapChipPos.x >= 0 && mapChipPos.x < static_cast<int>(map_[0].size()) &&
			mapChipPos.y >= 0 && mapChipPos.y < static_cast<int>(map_.size())) {

			if (buildMapChipIndex_.empty()) {
				buildMapChipIndex_.push_back(mapChipPos);
			} else if
				(buildMapChipIndex_.back().x != mapChipPos.x ||
					buildMapChipIndex_.back().y != mapChipPos.y) {
				buildMapChipIndex_.push_back(mapChipPos);
			}

			floorChip_.SetChipColor(mapChipPos.x, mapChipPos.y,{ 1.0f,0.0f,0.0f,1.0f });
		}
	}
	// 建築
	if (gamePlayer.GetIsBuilding()) {
		// 一個前の座標は削除
		buildMapChipIndex_.pop_back();
		for (const auto& index : buildMapChipIndex_) {
			if (map_[index.y][index.x] == 0) {
				map_[index.y][index.x] = 1;
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

		for (int y = 0; y < map_.size(); y++) {
			for (int x = 0; x < map_[y].size(); x++) {
				if (map_[y][x] != 0) {
					Vector2 mapChipPos = { static_cast<float>(x) * kBlockSize - (kBlockSize * 0.5f), static_cast<float>(y) * kBlockSize - (kBlockSize * 0.5f) };
					Vector2 playerPos = { gamePlayer.GetTransform().translate.x - 0.5f, gamePlayer.GetTransform().translate.z - 0.5f };
					if (MapChipCollider::IsAABBCollision(playerPos, 1.0f, 1.0f, mapChipPos, kBlockSize, kBlockSize)) {
						// 衝突法線を合成
						Vector2 playerCenter = playerPos + Vector2(0.5f, 0.5f);
						Vector2 blockCenter = mapChipPos + Vector2(kBlockSize * 0.5f, kBlockSize * 0.5f);
						Vector2 normal = (playerCenter - blockCenter).Normalize();
						totalNormal += normal;
						isCollided = true;
					}
				}
			}
		}

		// 合成法線で一度だけ反射
		if (isCollided && totalNormal.Length() > 0.0f) {
			Vector2 normal = totalNormal.Normalize();
			Vector2& velocity = gamePlayer.GetMoveDir();
			velocity = velocity - normal * (2.0f * Vector2::Dot(velocity, normal));

			gamePlayer.Jamp(-velocity);
		}
	}
}
void GameScene::GroundingUpdate(GamePlayer& gamePlayer) {
	if (gamePlayer.GetIsGrounded()) {
		IntVector2 mapChipPos;
		mapChipPos.x = static_cast<int>(std::round(gamePlayer.GetTransform().translate.x));
		mapChipPos.y = static_cast<int>(std::round(gamePlayer.GetTransform().translate.z));

		if (mapChipPos.x >= 0 && mapChipPos.x < static_cast<int>(map_[0].size()) &&
			mapChipPos.y >= 0 && mapChipPos.y < static_cast<int>(map_.size())) {

			// 周囲8マス＋中心の9マスを0にする
			for (int dy = -1; dy <= 1; ++dy) {
				for (int dx = -1; dx <= 1; ++dx) {
					int nx = mapChipPos.x + dx;
					int ny = mapChipPos.y + dy;
					if (nx >= 0 && nx < static_cast<int>(map_[0].size()) &&
						ny >= 0 && ny < static_cast<int>(map_.size())) {
						map_[ny][nx] = 0;
					}
				}
			}
		}
		gamePlayer.SetGrounded(false);
	}
}
