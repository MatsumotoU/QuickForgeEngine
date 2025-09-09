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

	mouseModel_ = std::make_unique<Model>(engineCore_, camera_);
	mouseModel_->LoadModel("Resources/Model/mouse", "mouse.obj", COORDINATESYSTEM_HAND_LEFT);
	mouseModel_.get()->transform_.translate = { 0.0f,0.0f,6.0f };
	mouseModel_.get()->transform_.rotate.x = 2.0f;
	mouseModel_.get()->SetDirectionalLightDir(directionalLightDir_);



	arrowModel_ = std::make_unique<Model>(engineCore_, camera_);
	arrowModel_->LoadModel("Resources/Model/TitleArrow", "TitleArrow.obj", COORDINATESYSTEM_HAND_LEFT);
	arrowModel_.get()->SetDirectionalLightDir(directionalLightDir_);

}

void TitleUI::Update(Mole* mole) {

	if (engineCore_->GetXInputController()->GetIsActiveController(0)) {
		input_ = engineCore_->GetXInputController()->GetLeftStick(0);
	}
	else if (engineCore_->GetInputManager()->mouse_.GetPress(0)) {

		Vector2 mousePos = engineCore_->GetInputManager()->mouse_.mouseScreenPos_;
		input_ = mousePos - mouseTargetPos;
	}
	else {
		input_ = { 0.0f,0.0f };
		mouseTargetPos = engineCore_->GetInputManager()->mouse_.mouseScreenPos_;
	}

	if (!input_.Length()) {
		model_.get()->transform_.translate = mole->GetTranslate();
		model_.get()->transform_.translate.y += 1.2f;
		model_.get()->transform_.rotate.z = roteta_;

		mouseModel_.get()->transform_.translate = mole->GetTranslate();
		mouseModel_.get()->transform_.translate.y += 1.2f;
		if (engineCore_->GetInputManager()->mouse_.GetPress(0)) {
			isRed_ = true;
			mouseModel_.get()->SetColor({ 1.0f,0.0f,0.0f,1.0f });
			mouseModel_.get()->transform_.rotate.z = roteta_;
		}
		else {
			if (colorFream_ % 60 == 0) {
				if (isRed_) {
					mouseModel_.get()->SetColor({ 1.0f,1.0f,1.0f,1.0f });
					isRed_ = false;
				}
				else {
					mouseModel_.get()->SetColor({ 1.0f,0.0f,0.0f,1.0f });
					isRed_ = true;
				}
			}
		}
		colorFream_++;
		roteta_ += rotetaSpeed_ / 60.0f;
		if (roteta_ <= -rotetoMax || roteta_ >= rotetoMax) {
			rotetaSpeed_ *= -1.0f;
		}

		isArrow_ = false;
		arrowModel_.get()->transform_.scale = { 0.0f,0.0f,0.0f };
		arrowDirection_ = None;
	}
	else {
		roteta_ = 0;
		colorFream_ = 0;
		isArrow_ = true;
		UpdateStickHold(mole);
	}

	model_.get()->Update();
	mouseModel_.get()->Update();
	arrowModel_.get()->Update();
}

void TitleUI::Draw(Mole* mole) {
	if (mole->GetMoleState() == Mole::MoleState::Normal &&
		!mole->ISAnimation()) {
		if (engineCore_->GetXInputController()->GetIsActiveController(0)) {
			model_.get()->Draw();
		}
		else {
			mouseModel_.get()->Draw();
		}
		if (isArrow_) {
			arrowModel_.get()->Draw();
		}
	}
}

void TitleUI::UpdateStickHold(Mole* mole)
{
	model_.get()->transform_.translate = mole->GetTranslate();
	model_.get()->transform_.translate.y += 1.2f;
	model_.get()->transform_.rotate.z = 0.0f;

	mouseModel_.get()->transform_.translate = mole->GetTranslate();
	mouseModel_.get()->transform_.translate.y += 1.2f;
	mouseModel_.get()->transform_.rotate.z = 0.0f;

	//矢印の処理
	arrowModel_.get()->transform_.translate = mole->GetTranslate();
	arrowModel_.get()->transform_.translate.y += 0.5f;

	if (mole->GetDirection() == Mole::Direction::Left) {
		if (arrowDirection_ != Left) {
			arrowSizeUPFlag_ = false;
			arrowModel_.get()->transform_.scale = { 0.0f,0.0f,0.0f };
		}
		model_.get()->transform_.translate.x -= 0.2f;
		mouseModel_.get()->transform_.translate.x -= 0.2f;
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
		mouseModel_.get()->transform_.translate.x += 0.2f;

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
