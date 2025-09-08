#include "TitleSkyDome.h"

void TitleSkyDome::DebugImGui()
{
	if (ImGui::CollapsingHeader("Skydome")) {
		ImGui::DragFloat3("SkydomeTranslate", &model_.get()->transform_.translate.x, 0.1f);
		ImGui::DragFloat3("SkydomeRotate", &model_.get()->transform_.rotate.x, 0.1f);
		ImGui::DragFloat3("SkydomeScale", &model_.get()->transform_.scale.x, 0.1f);
	}
}

void TitleSkyDome::Initialize(EngineCore* engineCore, Camera* camera) {
	engineCore_ = engineCore;
	camera_ = camera;

	model_ = std::make_unique<Model>(engineCore_, camera_);
	model_->LoadModel("Resources/Model/skydome", "skydome.obj", COORDINATESYSTEM_HAND_LEFT);
	model_.get()->SetDirectionalLightDir(directionalLightDir_);
	model_->SetEnableShadow(false);
}

void TitleSkyDome::Update() {
	model_.get()->transform_.rotate.y += (3.14f * 2.0f) / (60.0f * rotetaTime);
	model_->Update();
}

void TitleSkyDome::Draw() {
	model_->Draw();
}
