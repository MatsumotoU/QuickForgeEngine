#include "BigMole.h"

void BigMole::DebugImGui()
{
	ImGui::DragFloat3("WallTranslate", &model_.get()->transform_.translate.x, 0.1f);
	ImGui::DragFloat3("WallRotate", &model_.get()->transform_.rotate.x, 0.1f);
	ImGui::DragFloat3("WallScale", &model_.get()->transform_.scale.x, 0.1f);
	ImGui::DragFloat3("hangar_Translate", &hangar_.get()->transform_.translate.x, 0.1f);
	ImGui::DragFloat3("hangar_Rotate", &hangar_.get()->transform_.rotate.x, 0.1f);
	ImGui::DragFloat3("hangar_Scale", &hangar_.get()->transform_.scale.x, 0.1f);
}

void BigMole::Initialize(EngineCore* engineCore, Camera* camera) {
	engineCore_ = engineCore;
	camera_ = camera;

	model_ = std::make_unique<Model>(engineCore_, camera_);
	model_->LoadModel("Resources/Model/mole", "mole_fall.obj", COORDINATESYSTEM_HAND_LEFT);
	model_.get()->transform_.translate = { 0.0f,-6.2f,0.0f };
	model_.get()->transform_.scale = { 5.0f,5.0f,5.0f };
	model_.get()->SetColor({ 0.3f,0.3f,0.3f,1.0f });
	hangar_ = std::make_unique<Model>(engineCore_, camera_);
	hangar_->LoadModel("Resources/Model/hangar", "hangar.obj", COORDINATESYSTEM_HAND_LEFT);
	hangar_.get()->transform_.translate = { 0.0f,-6.2f,0.0f };
	hangar_.get()->transform_.scale = { 5.0f,5.0f,5.0f };
	hangar_.get()->SetColor({ 0.3f,0.3f,0.3f,1.0f });
}

void BigMole::Update() {
	hangar_->Update();
	model_->Update();
}

void BigMole::Draw() {
	hangar_->Draw();
	model_->Draw();
}