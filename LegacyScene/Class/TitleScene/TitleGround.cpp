#include "TitleGround.h"

void TitleGround::DebugImGui()
{
	if (ImGui::CollapsingHeader("TitleGround")) {
		ImGui::DragFloat3("TitleGroundTranslate", &model_.get()->transform_.translate.x, 0.1f);
		ImGui::DragFloat3("TitleGroundRotate", &model_.get()->transform_.rotate.x, 0.1f);
		ImGui::DragFloat3("TitleGroundScale", &model_.get()->transform_.scale.x, 0.1f);
	}
}

void TitleGround::Initialize(EngineCore* engineCore, Camera* camera, Vector3 directionalLightDir) {
	engineCore_ = engineCore;
	camera_ = camera;
	directionalLightDir_ = directionalLightDir;

	model_ = std::make_unique<Model>(engineCore_, camera_);
	model_->LoadModel("Resources/Model/titleMap", "titleMap.obj", COORDINATESYSTEM_HAND_LEFT);
	model_.get()->transform_.translate.y = -1.5f;
	model_.get()->SetDirectionalLightDir(directionalLightDir_);
}

void TitleGround::Update() {

	model_->Update();
}

void TitleGround::Draw() {
	model_->Draw();
}