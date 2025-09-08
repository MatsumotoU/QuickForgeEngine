#include "SignBoard.h"

void SignBoard::DebugImGui()
{
	if (ImGui::CollapsingHeader("SignBoard")) {
		ImGui::DragFloat3("SignBoardTranslate", &model_.get()->transform_.translate.x, 0.1f);
		ImGui::DragFloat3("SignBoardRotate", &model_.get()->transform_.rotate.x, 0.1f);
		ImGui::DragFloat3("SignBoardScale", &model_.get()->transform_.scale.x, 0.1f);
	}
}

void SignBoard::Initialize(EngineCore* engineCore, Camera* camera, Vector3 directionalLightDir) {
	engineCore_ = engineCore;
	camera_ = camera;
	directionalLightDir_ = directionalLightDir;

	model_ = std::make_unique<Model>(engineCore_, camera_);
	model_->LoadModel("Resources/Model/SignBoard", "SignBoard.obj", COORDINATESYSTEM_HAND_LEFT);
	model_.get()->transform_.translate = { 2.0f,0.0f,3.0f };
	model_.get()->transform_.rotate = { 0.0f,3.14f / 6,0.0f };
	//ここの二つのライト消してもらう

	letterModel_ = std::make_unique<Model>(engineCore_, camera_);
	letterModel_->LoadModel("Resources/Model/SignBoard", "SignBoardName.obj", COORDINATESYSTEM_HAND_LEFT);
	letterModel_.get()->transform_.translate = { 2.0f,0.0f,2.9f };
	letterModel_.get()->transform_.rotate = { 0.0f,3.14f/6,0.0f };
}

void SignBoard::Update() {

	model_->Update();
	letterModel_->Update();
}

void SignBoard::Draw() {
	model_->Draw();
	letterModel_->Draw();
}