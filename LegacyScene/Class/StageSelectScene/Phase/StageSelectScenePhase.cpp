#include "StageSelectScenePhase.h"
#include "../LegacyScene/StageSelectScene.h"
#include "../Object/Triangle.h"
#include "../Object/StageNumber.h"
#include "../Object/StageSelectBlocks.h"
#include "../System/CameraController.h"

void StageSelectScenePhaseIdle::Initialize() {
	stageSelectScene_->InitializeBlocks();
	stageSelectScene_->SetTriangleParent();
	stageSelectScene_->SetBlocksParent();
}

void StageSelectScenePhaseIdle::Update() {
	if (stageSelectScene_->GetDirectInput()->keyboard_.GetPress(DIK_A) ||
		stageSelectScene_->GetDirectInput()->keyboard_.GetPress(DIK_LEFT) ||
		stageSelectScene_->GetXInput()->GetLeftStick(0).x < 0.0f) {
		stageSelectScene_->CurrentStageDown();
		stageSelectScene_->SetDirection(Triangle::kLeft);
	} else if (stageSelectScene_->GetDirectInput()->keyboard_.GetPress(DIK_D) ||
		stageSelectScene_->GetDirectInput()->keyboard_.GetPress(DIK_RIGHT) ||
		stageSelectScene_->GetXInput()->GetLeftStick(0).x > 0.0f) {
		stageSelectScene_->CurrentStageUp();
		stageSelectScene_->SetDirection(Triangle::kRight);
	}

	stageSelectScene_->CurrentStageCircle();

	stageSelectScene_->GetCurrentStageObject()->Update();

	for (uint32_t i = 0; i < 2; ++i) {
		stageSelectScene_->GetTriangle(i)->Update();
	}

	for (uint32_t i = 0; i < 3; ++i) {
		stageSelectScene_->GetBlocks(i)->Update();
	}

	if (stageSelectScene_->GetDirectInput()->keyboard_.GetPress(DIK_A) ||
		stageSelectScene_->GetDirectInput()->keyboard_.GetPress(DIK_LEFT) ||
		stageSelectScene_->GetXInput()->GetLeftStick(0).x < 0.0f ||
		stageSelectScene_->GetDirectInput()->keyboard_.GetPress(DIK_D) ||
		stageSelectScene_->GetDirectInput()->keyboard_.GetPress(DIK_RIGHT) ||
		stageSelectScene_->GetXInput()->GetLeftStick(0).x > 0.0f) {
		stageSelectScene_->ChangePhase(new StageSelectScenePhasePush(stageSelectScene_));
	}

#ifdef _DEBUG
	ImGui::Text("CurrentScenePhase: Idle");
#endif // DEBUG
}

void StageSelectScenePhasePush::Initialize() {
	stageSelectScene_->GetTriangleByDirection()->SetFinished(false);
	stageSelectScene_->StopSelectSound();
	stageSelectScene_->PlaySelectSound();
}

void StageSelectScenePhasePush::Update() {
	stageSelectScene_->GetCurrentStageObject()->Update();

	stageSelectScene_->GetTriangleModelByDirection()->SetColor(Vector4{ 1.0f, 1.0f, 0.0f, 1.0f });

	stageSelectScene_->GetTriangleByDirection()->StateUpdate();

	for (uint32_t i = 0; i < 2; ++i) {
		stageSelectScene_->GetTriangle(i)->Update();
	}

	for (uint32_t i = 0; i < 3; ++i) {
		stageSelectScene_->GetBlocks(i)->Update();
	}

	if (stageSelectScene_->GetTriangleByDirection()->IsFinished()) {
		stageSelectScene_->ChangePhase(new StageSelectScenePhaseTransition(stageSelectScene_));
	}

#ifdef _DEBUG
	ImGui::Text("CurrentScenePhase: Push");
#endif // DEBUG
}

void StageSelectScenePhaseTransition::Initialize() {
	for (uint32_t i = 0; i < 2; ++i) {
		stageSelectScene_->GetTriangleModel(i)->SetColor(Vector4{ 1.0f, 1.0f, 1.0f, 1.0f });
	}

	stageSelectScene_->GetCurrentStageObject()->Reset();
	stageSelectScene_->GetCameraController()->Start();
	stageSelectScene_->SetCameraTargetPosition();
}

void StageSelectScenePhaseTransition::Update() {
	stageSelectScene_->GetCameraController()->Update();

	if (stageSelectScene_->GetCameraController()->IsFinished()) {
		stageSelectScene_->ChangePhase(new StageSelectScenePhaseIdle(stageSelectScene_));
	}

#ifdef _DEBUG
	ImGui::Text("CurrentScenePhase: Transition");
#endif // DEBUG
}