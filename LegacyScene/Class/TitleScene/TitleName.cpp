#include "TitleName.h"

void TitleName::DebugImGui()
{
	if (ImGui::CollapsingHeader("TitleName")) {
		ImGui::DragFloat3("TitleNameTranslate", &model_.get()->transform_.translate.x, 0.1f);
		ImGui::DragFloat3("TitleNameRotate", &model_.get()->transform_.rotate.x, 0.1f);
		ImGui::DragFloat3("TitleNameScale", &model_.get()->transform_.scale.x, 0.1f);
		
	}
}

void TitleName::Initialize(EngineCore* engineCore, Camera* camera) {
	engineCore_ = engineCore;
	camera_ = camera;

	model_ = std::make_unique<Model>(engineCore_, camera_);
	model_->LoadModel("Resources/Model/titleName", "titleName.obj", COORDINATESYSTEM_HAND_LEFT);
	model_.get()->transform_.translate = { 0.0f,0.6f,0.0f };
	model_.get()->SetDirectionalLightDir({ 0.0f,0.0f,9.0f });
}

void TitleName::Update() {

	model_->Update();
}

void TitleName::Draw() {
	model_->Draw();
}