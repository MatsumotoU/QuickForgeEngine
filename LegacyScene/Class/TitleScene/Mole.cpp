#include "Mole.h"

void Mole::DebugImGui()
{
	if (ImGui::CollapsingHeader("Mole")) {
		ImGui::DragFloat3("MoleTranslate", &model_.get()->transform_.translate.x,0.1f);
		ImGui::DragFloat3("MoleRotate", &model_.get()->transform_.rotate.x, 0.1f);
		ImGui::DragFloat3("MoleScale", &model_.get()->transform_.scale.x, 0.1f);
	}
}

void Mole::Initialize(EngineCore* engineCore, Camera* camera) {
	engineCore_ = engineCore;
	camera_ = camera;

	model_ = std::make_unique<Model>(engineCore_, camera_);
	model_->LoadModel("Resources/Model/mole", "mole.obj", COORDINATESYSTEM_HAND_LEFT);
	model_.get()->transform_.translate = { 1.3f,0.9f,1.0f };
	model_.get()->transform_.rotate = { 6.0f,2.5f,-1.1f };
	
}

void Mole::Update() {

	model_->Update();
}

void Mole::Draw() {
	model_->Draw();
}