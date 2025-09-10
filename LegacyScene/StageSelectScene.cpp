#include "StageSelectScene.h"
#include "TitleScene.h"
#include "GameScene.h"
#include "Class/StageSelectScene/Object/Triangle.h"
#include "Class/StageSelectScene/Object/StageSelectBlocks.h"
#include "Class/StageSelectScene/Object/StageSelectSkydome.h"
#include "Class/StageSelectScene/Object/StageNumber.h"
#include "Class/StageSelectScene/System/CameraController.h"
#include "Class/StageSelectScene/System/MapChipField.h"
#include "Class/StageSelectScene/System/StageSelectAnchor.h"
#include "Class/StageSelectScene/Phase/StageSelectScenePhase.h"
#include "../Engine/Particle/Particle.h"

StageSelectScene::StageSelectScene(EngineCore *engineCore, nlohmann::json *data) {
	engineCore_ = engineCore;
	directInput_ = engineCore_->GetInputManager();
	xInput_ = engineCore_->GetXInputController();
	engineCore_->GetGraphRenderer()->SetCamera(&camera_);
	sceneData_ = data;
}

StageSelectScene::~StageSelectScene() {
	delete currentPhase_;
	engineCore_->GetGraphRenderer()->DeleteCamera(&camera_);
	engineCore_->GetAudioPlayer()->StopAudio("ToGame.mp3");
	engineCore_->GetAudioPlayer()->StopAudio("ToTitle.mp3");
	engineCore_->GetAudioPlayer()->StopAudio("TurnChange.mp3");
	engineCore_->GetAudioPlayer()->StopAudio("StageSelect.mp3");
}

void StageSelectScene::Initialize() {
	// シーン切り替えフラグの初期化
	isRequestedExit_ = false;

	// カメラの初期化
	camera_.Initialize(engineCore_->GetWinApp());

	// 効果音の読み込み
	bgmHandle_ = engineCore_->LoadSoundData("Resources/Sound/BGM/", "StageSelect.mp3");
	selectSoundHandle_ = engineCore_->LoadSoundData("Resources/Sound/SE/", "TurnChange.mp3");
	toTitleSoundHandle_ = engineCore_->LoadSoundData("Resources/Sound/SE/", "ToTitle.mp3");
	toGameSoundHandle_ = engineCore_->LoadSoundData("Resources/Sound/SE/", "ToGame.mp3");
	engineCore_->GetAudioPlayer()->PlayAudio(bgmHandle_, "StageSelect.mp3", true);

	// 現在のステージの初期化
	if (sceneData_->contains("stage")) {
		currentStage_ = sceneData_->at("stage").get<int>();
	} else {
		currentStage_ = 0;
	}

	// ステージデータのリサイズ
	stageData_.stageMapChipFields.resize(kNumMapType);
	for (uint32_t i = 0; i < kNumMapType; ++i) {
		stageData_.stageMapChipFields[i].resize(StageSelectScene::kNumStage);
	}

	// ステージデータの読み込み
	for (uint32_t i = 0; i < StageSelectScene::kNumStage; ++i) {
		std::string wallFilePath = "Resources/Map/Stage" + std::to_string(i + 1) + "_wall.csv";
		std::string floorFilePath = "Resources/Map/Stage" + std::to_string(i + 1) + "_floor.csv";
		stageData_.stageMapChipFields[kWall][i].LoadMapChipCsv(wallFilePath);
		stageData_.stageMapChipFields[kFloor][i].LoadMapChipCsv(floorFilePath);
	}

	// 天球モデルの初期化
	skydomeModel_ = std::make_unique<Model>(engineCore_, &camera_);
	skydomeModel_->LoadModel("Resources/Model/skydome", "skydome.obj", COORDINATESYSTEM_HAND_LEFT);

	// 大きなモグラモデルの初期化
	bigMoleModel_ = std::make_unique<Model>(engineCore_, &camera_);
	bigMoleModel_->LoadModel("Resources/Model/mole", "mole_fall.obj", COORDINATESYSTEM_HAND_LEFT);

	// 格納庫モデルの初期化
	hangarModel_ = std::make_unique<Model>(engineCore_, &camera_);
	hangarModel_->LoadModel("Resources/Model/hangar", "hangar.obj", COORDINATESYSTEM_HAND_LEFT);

	// ブロックパーティクルの初期化
	uint32_t size = MapChipField::kNumBlockHorizontal * MapChipField::kNumBlockVertical * StageSelectScene::kNumMapType;
	for (uint32_t i = 0; i < blockParticles_.size(); ++i) {
		blockParticles_[i] = std::make_unique<Particle>();
		blockParticles_[i]->Initialize(engineCore_, size);
	}
	blockParticles_[static_cast<uint32_t>(MapChipType::kDirt) - 1]->LoadModel("Resources/Model/blocks/dirt", "dirt.obj", COORDINATESYSTEM_HAND_LEFT);
	blockParticles_[static_cast<uint32_t>(MapChipType::kGrass) - 1]->LoadModel("Resources/Model/blocks/grass", "grass.obj", COORDINATESYSTEM_HAND_LEFT);
	blockParticles_[static_cast<uint32_t>(MapChipType::kStone) - 1]->LoadModel("Resources/Model/blocks/stone", "stone.obj", COORDINATESYSTEM_HAND_LEFT);

	// 三角錐モデルの初期化
	for (uint32_t i = 0; i < triangleModels_.size(); ++i) {
		triangleModels_[i] = std::make_unique<Model>(engineCore_, &camera_);
		triangleModels_[i]->LoadModel("Resources/Model/blocks", "triangle.obj", COORDINATESYSTEM_HAND_LEFT);
	}

	// ステージモデルの初期化
	stageNumberModels_.resize(kNumStage);
	for (uint32_t i = 0; i < stageNumberModels_.size(); ++i) {
		stageNumberModels_[i] = std::make_unique<Model>(engineCore_, &camera_);
		stageNumberModels_[i]->LoadModel("Resources/Model/UI", std::to_string(i + 1) + ".obj", COORDINATESYSTEM_HAND_LEFT);
	}

	// アンカーの初期化
	for (uint32_t i = 0; i < anchor_.size(); ++i) {
		anchor_[i] = std::make_unique<StageSelectAnchor>();
		anchor_[i]->Initialize(i);
	}

	// 天球の初期化
	skydome_ = std::make_unique<StageSelectSkydome>();
	skydome_->Initialize(skydomeModel_.get());

	// 大きなモグラの初期化
	bigMole_ = std::make_unique<BigMole>();
	bigMole_->Initialize(bigMoleModel_.get(), hangarModel_.get());

	// ブロックの初期化
	for (uint32_t i = 0; i < blocks_.size(); ++i) {
		blocks_[i] = std::make_unique<StageSelectBlocks>(blockParticles_[i].get(), &camera_);
	}

	// 三角錐の初期化
	for (uint32_t i = 0; i < triangles_.size(); ++i) {
		triangles_[i] = std::make_unique<Triangle>();
		triangles_[i]->Initialize(triangleModels_[i].get(), directInput_, xInput_, static_cast<Triangle::Direction>(i));
	}

	// ステージオブジェクトの初期化
	for (uint32_t i = 0; i < stageNumbers_.size(); ++i) {
		stageNumbers_[i] = std::make_unique<StageNumber>();
		stageNumbers_[i]->Initialize(stageNumberModels_[i].get(), anchor_[i]->GetWorldMatrix());
	}

	// カメラコントローラーの初期化
	cameraController_ = std::make_unique<CameraController>();
	cameraController_->Initialize(&camera_, anchor_[currentStage_]->GetWorldPosition());

	// 現在のフェーズの初期化
	currentPhase_ = new StageSelectScenePhaseIdle(this);
	currentPhase_->Initialize();
}

void StageSelectScene::Update() {
	// フレームカウント
	frameCount_++;

	// シーン切り替え
	if (directInput_->keyboard_.GetTrigger(DIK_SPACE) || xInput_->GetTriggerButton(XINPUT_GAMEPAD_A, 0)) {
		isRequestedExit_ = true;
		transitionState_ = ToGame;
		engineCore_->GetAudioPlayer()->PlayAudio(toGameSoundHandle_, "ToGame.mp3", false);
	} else if (directInput_->keyboard_.GetTrigger(DIK_ESCAPE) || xInput_->GetTriggerButton(XINPUT_GAMEPAD_B, 0)) {
		isRequestedExit_ = true;
		transitionState_ = ToTitle;
		engineCore_->GetAudioPlayer()->PlayAudio(toTitleSoundHandle_, "ToTitle.mp3", false);
	}

	// 天球の更新
	skydome_->Update();
	//大きいモグラの更新
	bigMole_->Update();
	// 現在のフェーズの更新
	currentPhase_->Update();
}

void StageSelectScene::Draw() {
	// 天球の描画
	skydome_->Draw();
	//大きいモグラの描画
	bigMole_->Draw();


	// 三角錐の描画
	for (auto &triangle : triangles_) {
		triangle->Draw();
	}

	// ステージオブジェクトの描画
	for (auto &stageNumber : stageNumbers_) {
		stageNumber->Draw();
	}

	// ブロックの描画
	for (auto &block : blocks_) {
		block->Draw();
	}

}

IScene *StageSelectScene::GetNextScene() {
	(*sceneData_)["stage"] = currentStage_;

	switch (transitionState_) {
		case StageSelectScene::None:
			break;
		case StageSelectScene::ToGame:
			return new GameScene(engineCore_, sceneData_);
			break;
		case StageSelectScene::ToTitle:
			return new TitleScene(engineCore_, sceneData_);
			break;
		default:
			break;
	}
	return this;
}

void StageSelectScene::CameraUpdate() {

}

void StageSelectScene::InitializeBlocks() {
	for (uint32_t i = 0; i < blocks_.size(); ++i) {
		blocks_[i]->Initialize(stageData_, currentStage_, static_cast<MapChipType>(i + 1));
	}
}

void StageSelectScene::ChangePhase(BaseStageSelectScenePhase *newPhase) {
	delete currentPhase_;
	currentPhase_ = newPhase;
	currentPhase_->Initialize();
}

void StageSelectScene::SetTriangleParent() {
	for (auto &triangle : triangles_) {
		triangle->SetParentWorldMatrix(anchor_[currentStage_]->GetWorldMatrix());
	}
}

void StageSelectScene::SetBlocksParent() {
	for (auto &block : blocks_) {
		block->SetParentWorldMatrix(anchor_[currentStage_]->GetWorldMatrix());
	}
}

void StageSelectScene::SetCameraTargetPosition() {
	cameraController_->SetTargetPosition(anchor_[currentStage_]->GetWorldPosition());
}