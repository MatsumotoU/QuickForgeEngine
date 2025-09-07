#include "StageSelectScene.h"
#include "TitleScene.h"
#include "GameScene.h"
#include "Class/StageSelectScene/Object/Triangle.h"
#include "Class/StageSelectScene/Object/StageObject.h"
#include "Class/StageSelectScene/System/CameraController.h"
#include "Class/StageSelectScene/Phase/StageSelectScenePhase.h"

StageSelectScene::StageSelectScene(EngineCore *engineCore, nlohmann::json* data) : debugCamera_(engineCore) {
	engineCore_ = engineCore;
	directInput_ = engineCore_->GetInputManager();
	xInput_ = engineCore_->GetXInputController();
	engineCore_->GetGraphRenderer()->SetCamera(&camera_);

	sceneData_ = data;
}

StageSelectScene::~StageSelectScene() {
	delete currentPhase_;
	engineCore_->GetGraphRenderer()->DeleteCamera(&camera_);
}

void StageSelectScene::Initialize() {
#ifdef _DEBUG
	// デバッグカメラの初期化
	isActiveDebugCamera_ = false;
	debugCamera_.Initialize(engineCore_);
	debugCamera_.camera_.transform_.translate.z = -20.0f;
#endif // _DEBUG

	// シーン切り替えフラグの初期化
	isRequestedExit_ = false;

	// カメラの初期化
	camera_.Initialize(engineCore_->GetWinApp());

	// 三角錐モデルの初期化
	for(uint32_t i = 0; i < triangleModels_.size(); ++i) {
		triangleModels_[i] = std::make_unique<Model>(engineCore_, &camera_);
		triangleModels_[i]->LoadModel("Resources", "triangle.obj", COORDINATESYSTEM_HAND_LEFT);
	}

	// ステージモデルの初期化
	stageModels_.resize(kNumStage);
	for (uint32_t i = 0; i < stageModels_.size(); ++i) {
		stageModels_[i] = std::make_unique<Model>(engineCore_, &camera_);
		stageModels_[i]->LoadModel("Resources", std::to_string(i) + ".obj", COORDINATESYSTEM_HAND_LEFT);
	}

	// 三角錐の初期化
	for(uint32_t i = 0; i < triangles_.size(); ++i) {
		triangles_[i] = std::make_unique<Triangle>();
		triangles_[i]->Initialize(triangleModels_[i].get(), directInput_, xInput_, static_cast<Triangle::Direction>(i));
	}

	// ステージオブジェクトの初期化
	stageObjects_.resize(kNumStage);
	for (uint32_t i = 0; i < stageObjects_.size(); ++i) {
		stageObjects_[i] = std::make_unique<StageObject>();
		stageObjects_[i]->Initialize(stageModels_[i].get(), i);
		stageObjects_[i]->Update();
	}

	// カメラコントローラーの初期化
	cameraController_ = std::make_unique<CameraController>();
	cameraController_->Initialize(&camera_, stageModels_[currentStage_]->GetWorldPosition());

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
	} else if (directInput_->keyboard_.GetTrigger(DIK_ESCAPE) || xInput_->GetTriggerButton(XINPUT_GAMEPAD_B, 0)) {
		isRequestedExit_ = true;
		transitionState_ = ToTitle;
	}

#ifdef _DEBUG
	// デバッグカメラの切り替え
	if (directInput_->keyboard_.GetTrigger(DIK_P)) {
		isActiveDebugCamera_ = !isActiveDebugCamera_;
	}

	// カメラの更新
	CameraUpdate();

	ImGui::Text("CurrentStage: %d", currentStage_);
#endif // _DEBUG

	// 現在のフェーズの更新
	currentPhase_->Update();
}

void StageSelectScene::Draw() {
	// グリッドの描画
	engineCore_->GetGraphRenderer()->DrawGrid();

#ifdef _DEBUG
	// デバッグカメラのImGui描画
	debugCamera_.DrawImGui();
#endif // _DEBUG

	// 三角錐の描画
	for (auto &triangle : triangles_) {
		triangle->Draw();
	}

	// ステージオブジェクトの描画
	for (auto &stageObject : stageObjects_) {
		stageObject->Draw();
	}
}

IScene *StageSelectScene::GetNextScene() {
	(*sceneData_)["stage"] = currentStage_;

	switch (transitionState_) {
		case StageSelectScene::None:
			break;
		case StageSelectScene::ToGame:
			return new GameScene(engineCore_,sceneData_);
			break;
		case StageSelectScene::ToTitle:
			return new TitleScene(engineCore_,sceneData_);
			break;
		default:
			break;
	}
	return this;
}

void StageSelectScene::CameraUpdate() {
#ifdef _DEBUG
	if (isActiveDebugCamera_) {
		debugCamera_.Update();
		camera_ = debugCamera_.camera_;
	}
#endif // _DEBUG
}

void StageSelectScene::ChangePhase(BaseStageSelectScenePhase *newPhase) {
	delete currentPhase_;
	currentPhase_ = newPhase;
	currentPhase_->Initialize();
}

void StageSelectScene::SetTriangleParent() {
	for (auto &triangle : triangles_) {
		triangle->SetParent(stageModels_[currentStage_]->worldMatrix_);
	}
}

void StageSelectScene::SetCameraTargetPosition() {
	cameraController_->SetTargetPosition(stageModels_[currentStage_]->GetWorldPosition());
}