#include "SignBoard.h"

void SignBoard::DebugImGui()
{
	if (ImGui::CollapsingHeader("Wall")) {
		ImGui::DragFloat3("WallTranslate", &model_.get()->transform_.translate.x, 0.1f);
		ImGui::DragFloat3("WallRotate", &model_.get()->transform_.rotate.x, 0.1f);
		ImGui::DragFloat3("WallScale", &model_.get()->transform_.scale.x, 0.1f);
	}
}

void SignBoard::Initialize(EngineCore* engineCore, Camera* camera) {
	engineCore_ = engineCore;
	camera_ = camera;

	model_ = std::make_unique<Model>(engineCore_, camera_);
	model_->LoadModel("Resources/Model/SignBoard", "SignBoard.obj", COORDINATESYSTEM_HAND_LEFT);
	model_.get()->transform_.translate = { 2.0f,0.0f,2.0f };
	model_.get()->transform_.scale = { 1.0f,1.0f,1.0f };
}

void SignBoard::Update() {

	model_->Update();
}

void SignBoard::Draw() {
	model_->Draw();
}