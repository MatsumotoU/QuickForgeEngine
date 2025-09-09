#include "ContorolUI.h"
#include "Utility/MyEasing.h"

void ControlUI::Initialize(EngineCore* engineCore, Camera* camera) {
	engineCore_ = engineCore;
	camera_ = camera;

	controlTextModel_ = std::make_unique<Model>(engineCore, camera);
	controlTextModel_->LoadModel("Resources/Model/UI", "controlTextobj.obj", COORDINATESYSTEM_HAND_LEFT);
	controllerModel_ = std::make_unique<Model>(engineCore, camera);
	controllerModel_->LoadModel("Resources/Model/LStickUI", "LStickUI.obj", COORDINATESYSTEM_HAND_LEFT);
	mouseModel_ = std::make_unique<Model>(engineCore, camera);
	mouseModel_->LoadModel("Resources/Model/mouse", "mouse.obj", COORDINATESYSTEM_HAND_LEFT);

	transform_.translate = { -1.0f,2.8f,-0.3f };
	transform_.rotate = { 1.3f,0.0f,0.0f };

	clickPos_ = { 0.0f,0.0f };
}

void ControlUI::Update() {
	MyEasing::SimpleEaseIn(&transform_.scale.x, 1.0f, 0.1f);
	MyEasing::SimpleEaseIn(&transform_.scale.y, 1.0f, 0.1f);

	timer_ += engineCore_->GetDeltaTime();
	scalingTimer_ += engineCore_->GetDeltaTime();

	controlTextModel_->transform_ = transform_;
	controllerModel_->transform_ = transform_;
	mouseModel_->transform_ = transform_;

	controlTextModel_->transform_.translate.x += 1.5f;
	controllerModel_->transform_.translate.x -= 1.0f;
	mouseModel_->transform_.translate.x -= 1.0f;

	controllerModel_->transform_.scale = { 2.0f,2.0f,2.0f };
	controllerModel_->transform_.rotate.x = 3.14f;
	mouseModel_->transform_.scale = { 2.0f,2.0f,2.0f };
	mouseModel_->transform_.rotate.x = 3.14f;

	if (isActiveControll_) {
		if (scalingTimer_ >= 0.8f) {
			scalingTimer_ = 0.0f;
			transform_.scale.x = 0.8f;
			transform_.scale.y = 0.8f;
		}

		controllerModel_->SetColor({ 1.0f,1.0f,1.0f,1.0f });
		if (engineCore_->GetXInputController()->GetLeftStick(0).Length() == 0.0f) {
			controllerModel_->transform_.rotate.x = 3.14f + sinf(timer_ * 5.0f) * 0.1f;
			controllerModel_->transform_.rotate.y = 0.0f + cosf(timer_ * 5.0f) * 0.1f;

		} else {
			Vector2 leftStick = engineCore_->GetXInputController()->GetLeftStick(0).Normalize();
			controllerModel_->transform_.rotate.x = 3.14f + leftStick.y * 0.5f;
			controllerModel_->transform_.rotate.z = 0.0f + -leftStick.x * 0.5f;
			controllerModel_->SetColor({ 1.0f,0.0f,0.0f,1.0f });

		}

		if (engineCore_->GetInputManager()->mouse_.GetTrigger(0)) {
			isClicked_ = true;
			clickPos_ = engineCore_->GetInputManager()->mouse_.mouseScreenPos_;
		}
		Vector2 dragDir;
		if (engineCore_->GetInputManager()->mouse_.GetPress(0)) {
			// ドラッグ中の処理
			if (isClicked_) {
				Vector2 currentMousePos = engineCore_->GetInputManager()->mouse_.mouseScreenPos_;
				Vector2 dragVector = clickPos_ - currentMousePos;
				dragVector.x *= -1.0f;

				dragDir = -dragVector.Normalize();
				mouseModel_->SetColor({ 1.0f,0.0f,0.0f,1.0f });
				mouseModel_->transform_.translate.x += -dragDir.x * 0.5f;
				mouseModel_->transform_.translate.z += -dragDir.y * 0.5f;
			}
		} else {
			mouseModel_->transform_.translate.x += sinf(timer_ * 5.0f) * 0.1f;
			mouseModel_->transform_.translate.z += cosf(timer_ * 5.0f) * 0.1f;
		}

		if (engineCore_->GetInputManager()->mouse_.GetRelease(0)) {
			if (isClicked_) {
				isClicked_ = false;
				Vector2 clickEndPos = engineCore_->GetInputManager()->mouse_.mouseScreenPos_;
				Vector2 dragVector = clickPos_ - clickEndPos;
				mouseModel_->SetColor({ 1.0f,1.0f,1.0f,1.0f });

			}
		}

	} else {
		controllerModel_->SetColor({ 0.5f,0.5f,0.5f,1.0f });
		mouseModel_->SetColor({ 0.5f,0.5f,0.5f,1.0f });
	}



	controlTextModel_->Update();
	controllerModel_->Update();
	mouseModel_->Update();

}

void ControlUI::Draw() {
	controlTextModel_->Draw();
	if (engineCore_->GetXInputController()->GetIsActiveController(0)) {
		controllerModel_->Draw();
	} else {
		mouseModel_->Draw();
	}
}
