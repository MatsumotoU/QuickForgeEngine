#include "TitleName.h"

void TitleName::DebugImGui()
{
	if (ImGui::CollapsingHeader("TitleName")) {
		ImGui::DragFloat3("TitleNameTranslate", &model_.get()->transform_.translate.x, 0.1f);
		ImGui::DragFloat3("TitleNameRotate", &model_.get()->transform_.rotate.x, 0.1f);
		ImGui::DragFloat3("TitleNameScale", &model_.get()->transform_.scale.x, 0.1f);
		
	}
}

void TitleName::Initialize(EngineCore* engineCore, Camera* camera, Vector3 directionalLightDir) {
	engineCore_ = engineCore;
	camera_ = camera;
	directionalLightDir_ = directionalLightDir;

	model_ = std::make_unique<Model>(engineCore_, camera_);
	model_->LoadModel("Resources/Model/titleName", "titleName.obj", COORDINATESYSTEM_HAND_LEFT);
	model_.get()->transform_.translate = { 0.0f,0.6f,0.0f };
	model_.get()->transform_.rotate = { 0.2f,-0.2f,0.0f }; 
	model_.get()->SetDirectionalLightDir(directionalLightDir_);
}

void TitleName::Update() {
	time+=3.0f/60.0f; // 経過時間
	float y = 0.05f * sinf(time) + 0.6f;
	model_.get()->transform_.translate.y = y;
	model_->Update();
}

void TitleName::Draw() {
	model_->Draw();
}