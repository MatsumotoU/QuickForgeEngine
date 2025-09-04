#include "Wall.h"

void Wall::DebugImGui()
{
	if (ImGui::CollapsingHeader("Wall")) {
		ImGui::DragFloat3("MoleTranslate", &model_.get()->transform_.translate.x, 0.1f);
		ImGui::DragFloat3("MoleRotate", &model_.get()->transform_.rotate.x, 0.1f);
		ImGui::DragFloat3("MoleScale", &model_.get()->transform_.scale.x, 0.1f);
	}
}

void Wall::Initialize(EngineCore* engineCore, Camera* camera) {
	engineCore_ = engineCore;
	camera_ = camera;

	model_ = std::make_unique<Model>(engineCore_, camera_);
	model_->LoadModel("Resources/Model/wall/soil", "soil.obj", COORDINATESYSTEM_HAND_LEFT);
	model_.get()->transform_.translate = { -1.5f,-0.6f,1.0f };
}

void Wall::Update() {

	model_->Update();
}

void Wall::Draw() {
	model_->Draw();
}