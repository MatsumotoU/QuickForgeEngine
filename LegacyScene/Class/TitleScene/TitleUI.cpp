#include "TitleUI.h"

#include "Mole.h"

void TitleUI::DebugImGui()
{
	if (ImGui::CollapsingHeader("TitleUI")) {
		ImGui::DragFloat3("TitleUITranslate", &model_.get()->transform_.translate.x, 0.1f);
		ImGui::DragFloat3("TitleUIRotate", &model_.get()->transform_.rotate.x, 0.1f);
		ImGui::DragFloat3("TitleUIScale", &model_.get()->transform_.scale.x, 0.1f);
	}
}

void TitleUI::Initialize(EngineCore* engineCore, Camera* camera, Vector3 directionalLightDir) {
	engineCore_ = engineCore;
	camera_ = camera;
	directionalLightDir_ = directionalLightDir;

	model_ = std::make_unique<Model>(engineCore_, camera_);
	model_->LoadModel("Resources/Model/LStickUI", "LStickUI.obj", COORDINATESYSTEM_HAND_LEFT);
	model_.get()->transform_.translate = { 0.0f,0.0f,6.0f };
	model_.get()->transform_.rotate.x = 2.0f;
	model_.get()->SetDirectionalLightDir(directionalLightDir_);
}

void TitleUI::Update(Mole* mole) {

	model_.get()->transform_.translate = mole->GetTranslate();
	model_.get()->transform_.translate.y += 1.2f;

	model_.get()->transform_.rotate.z = roteta_;
	roteta_ += speed_ / 60.0f;
	if (roteta_ <= -rotetoMax || roteta_ >= rotetoMax) {
		speed_ *= -1.0f;
	}


	model_->Update();
}

void TitleUI::Draw(Mole* mole) {
	if (mole->GetMoleState() == Mole::MoleState::Normal) {
		model_->Draw();
	}
}