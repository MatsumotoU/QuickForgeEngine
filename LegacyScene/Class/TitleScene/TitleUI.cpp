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

	arrowModel_ = std::make_unique<Model>(engineCore_, camera_);
	arrowModel_->LoadModel("Resources/Model/TitleArrow", "TitleArrow.obj", COORDINATESYSTEM_HAND_LEFT);
	arrowModel_.get()->SetDirectionalLightDir(directionalLightDir_);

}

void TitleUI::Update(Mole* mole) {

	leftStick_ = engineCore_->GetXInputController()->GetLeftStick(0);
	if (!leftStick_.Length()) {
		model_.get()->transform_.translate = mole->GetTranslate();
		model_.get()->transform_.translate.y += 1.2f;
		model_.get()->transform_.rotate.z = roteta_;
		roteta_ += speed_ / 60.0f;
		if (roteta_ <= -rotetoMax || roteta_ >= rotetoMax) {
			speed_ *= -1.0f;
		}
		isArrow_ = false;
		arrowModel_.get()->transform_.scale = { 0.0f,0.0f,0.0f };
		arrowDirection_ = None;
	}
	else {
		isArrow_ = true;
		UpdateStickHold(mole);
	}

	model_.get()->Update();
	arrowModel_.get()->Update();
}

void TitleUI::Draw(Mole* mole) {
	if (mole->GetMoleState() == Mole::MoleState::Normal&&
		!mole->ISAnimation()) {
		model_.get()->Draw();
		if (isArrow_) {
			arrowModel_.get()->Draw();
		}
	}
}

void TitleUI::UpdateStickHold(Mole* mole)
{
	model_.get()->transform_.translate = mole->GetTranslate();
	model_.get()->transform_.translate.y += 1.2f;

	//矢印の処理
	arrowModel_.get()->transform_.translate = mole->GetTranslate();
	arrowModel_.get()->transform_.translate.y += 0.5f;

	if (mole->GetDirection() == Mole::Direction::Left) {
		if (arrowDirection_ != Left) {
			arrowSizeUPFlag_ = false;
			arrowModel_.get()->transform_.scale = { 0.0f,0.0f,0.0f };
		}
		model_.get()->transform_.translate.x -= 0.2f;
		//矢印の処理
		arrowModel_.get()->transform_.translate.x += 1.0f;
		arrowModel_.get()->transform_.rotate.z = 3.14f;
		ArrowSizeUP();
		arrowDirection_ = Left;
	}
	if (mole->GetDirection() == Mole::Direction::Right) {
		if (arrowDirection_ != Right) {
			arrowSizeUPFlag_ = false;
			arrowModel_.get()->transform_.scale = { 0.0f,0.0f,0.0f };
		}
		model_.get()->transform_.translate.x += 0.2f;
		//矢印の処理
		arrowModel_.get()->transform_.translate.x -= 1.0f;
		arrowModel_.get()->transform_.rotate.z = 0;
		ArrowSizeUP();
		arrowDirection_ = Right;
	}
}

void TitleUI::ArrowSizeUP()
{
	if (!arrowSizeUPFlag_) {
		arrowModel_.get()->transform_.scale += arrowSizeUPSpeed_;

	}if (arrowModel_.get()->transform_.scale.x >= 1.6f) {
		arrowModel_.get()->transform_.scale = { 1.0f,1.0f,1.0f };
		arrowSizeUPFlag_ = true;
	}
}
