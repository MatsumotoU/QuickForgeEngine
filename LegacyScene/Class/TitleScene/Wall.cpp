#include "Wall.h"

void Wall::DebugImGui()
{
	if (ImGui::CollapsingHeader("Wall")) {
		ImGui::DragFloat3("WallTranslate", &model_.get()->transform_.translate.x, 0.1f);
		ImGui::DragFloat3("WallRotate", &model_.get()->transform_.rotate.x, 0.1f);
		ImGui::DragFloat3("WallScale", &model_.get()->transform_.scale.x, 0.1f);
	}
}

void Wall::Initialize(EngineCore* engineCore, Camera* camera, Vector3 directionalLightDir) {
	engineCore_ = engineCore;
	camera_ = camera;
	directionalLightDir_ = directionalLightDir;

	model_ = std::make_unique<Model>(engineCore_, camera_);
	model_->LoadModel("Resources/Model/wall/wood", "wood.obj", COORDINATESYSTEM_HAND_LEFT);
	model_.get()->transform_.translate = { 0.0f,0.0f,4.0f };
	model_.get()->transform_.scale = { 2.0f,2.0f,2.0f };
	model_.get()->SetDirectionalLightDir(directionalLightDir_);
}

void Wall::Update() {

	model_->Update();
}

void Wall::Draw() {
	model_->Draw();
}