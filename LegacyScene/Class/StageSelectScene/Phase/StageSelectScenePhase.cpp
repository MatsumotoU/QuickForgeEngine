#include "StageSelectScenePhase.h"
#include "../LegacyScene/StageSelectScene.h"
#include "../Object/Triangle.h"
#include "../Object/StageObject.h"
#include "../System/CameraController.h"

void StageSelectScenePhaseIdle::Initialize() {
	stageSelectScene_->SetTriangleParent();
}

void StageSelectScenePhaseIdle::Update() {
	if (stageSelectScene_->GetInput()->keyboard_.GetPress(DIK_A) || stageSelectScene_->GetInput()->keyboard_.GetPress(DIK_LEFT)) {
		stageSelectScene_->CurrentStageDown();
		direction_ = Triangle::Direction::kLeft;
	} else if (stageSelectScene_->GetInput()->keyboard_.GetPress(DIK_D) || stageSelectScene_->GetInput()->keyboard_.GetPress(DIK_RIGHT)) {
		stageSelectScene_->CurrentStageUp();
		direction_ = Triangle::Direction::kRight;
	}

	stageSelectScene_->CurrentStageCircle();

	stageSelectScene_->GetCurrentStageObject()->Update();

	for (uint32_t i = 0; i < 2; ++i) {
		stageSelectScene_->GetTriangle(i)->Update();
	}

	if (stageSelectScene_->GetInput()->keyboard_.GetPress(DIK_A) || stageSelectScene_->GetInput()->keyboard_.GetPress(DIK_LEFT) ||
		stageSelectScene_->GetInput()->keyboard_.GetPress(DIK_D) || stageSelectScene_->GetInput()->keyboard_.GetPress(DIK_RIGHT)) {
		stageSelectScene_->ChangePhase(new StageSelectScenePhasePush(stageSelectScene_, direction_));
	}

#ifdef _DEBUG
	ImGui::Text("CurrentaPhase: Idle");
#endif // DEBUG
}

void StageSelectScenePhasePush::Initialize() {
	stageSelectScene_->GetTriangle(static_cast<uint32_t>(direction_))->SetFinished(false);
}

void StageSelectScenePhasePush::Update() {
	stageSelectScene_->GetCurrentStageObject()->Update();

	stageSelectScene_->GetTriangle(static_cast<uint32_t>(direction_))->StateUpdate();

	for (uint32_t i = 0; i < 2; ++i) {
		stageSelectScene_->GetTriangle(i)->Update();
	}

	if (stageSelectScene_->GetTriangle(static_cast<uint32_t>(direction_))->IsFinished()) {
		stageSelectScene_->ChangePhase(new StageSelectScenePhaseTransition(stageSelectScene_, direction_));
	}

#ifdef _DEBUG
	ImGui::Text("CurrentaPhase: Push");
#endif // DEBUG
}

void StageSelectScenePhaseTransition::Initialize() {
	stageSelectScene_->GetCurrentStageObject()->Reset();
	stageSelectScene_->GetCameraController()->Start();
	stageSelectScene_->SetCameraTargetPosition();
}

void StageSelectScenePhaseTransition::Update() {
	stageSelectScene_->GetCameraController()->Update();

	if (stageSelectScene_->GetCameraController()->IsFinished()) {
		stageSelectScene_->ChangePhase(new StageSelectScenePhaseIdle(stageSelectScene_, direction_));
	}

#ifdef _DEBUG
	ImGui::Text("CurrentaPhase: Transition");
#endif // DEBUG
}