#include "BigMole.h"
#include "../Engine/Model/Model.h"

void BigMole::DebugImGui() {
	ImGui::DragFloat3("WallTranslate", &model_->transform_.translate.x, 0.1f);
	ImGui::DragFloat3("WallRotate", &model_->transform_.rotate.x, 0.1f);
	ImGui::DragFloat3("WallScale", &model_->transform_.scale.x, 0.1f);
	ImGui::DragFloat3("hangar_Translate", &hangar_->transform_.translate.x, 0.1f);
	ImGui::DragFloat3("hangar_Rotate", &hangar_->transform_.rotate.x, 0.1f);
	ImGui::DragFloat3("hangar_Scale", &hangar_->transform_.scale.x, 0.1f);
}

void BigMole::Initialize(Model *model, Model *hangar) {
	model_ = model;
	hangar_ = hangar;
	model_->transform_.translate = { 0.0f,-6.2f,0.0f };
	model_->transform_.scale = { 5.0f,5.0f,5.0f };
	model_->SetColor({ 0.3f,0.3f,0.3f,1.0f });
	hangar_->transform_.translate = { 0.0f,-6.2f,0.0f };
	hangar_->transform_.scale = { 5.0f,5.0f,5.0f };
	hangar_->SetColor({ 0.3f,0.3f,0.3f,1.0f });
}

void BigMole::Update() {
	hangar_->Update();
	model_->Update();
}

void BigMole::Draw() {
	hangar_->Draw();
	model_->Draw();
}