#include "KeyTutorial.h"
#include "Utility/MyEasing.h"

void KeyTutorial::Initialize(EngineCore* engineCore) {
	engineCore_ = engineCore;
	isActive_ = false;
	keyTutorialModel_.Initialize(engineCore_);
	buttonTutorialModel_.Initialize(engineCore_);

	keyTutorialModel_.LoadModel("Resources/", "KeyTutorial.obj", COORDINATESYSTEM_HAND_RIGHT);
	buttonTutorialModel_.LoadModel("Resources/", "ContorollerTutorial.obj", COORDINATESYSTEM_HAND_RIGHT);

	transform_.rotate = { 0.0f,3.14f,0.0f };
}

void KeyTutorial::Update() {
	if (isActive_) {
		Eas::SimpleEaseIn(&transform_.scale.x, 1.0f, 0.1f);
		Eas::SimpleEaseIn(&transform_.scale.y, 1.0f, 0.1f);
		Eas::SimpleEaseIn(&transform_.scale.z, 1.0f, 0.1f);
	} else {
		Eas::SimpleEaseIn(&transform_.scale.x, 0.0f, 0.1f);
		Eas::SimpleEaseIn(&transform_.scale.y, 0.0f, 0.1f);
		Eas::SimpleEaseIn(&transform_.scale.z, 0.0f, 0.1f);
	}

	keyTutorialModel_.transform_ = transform_;
	buttonTutorialModel_.transform_ = transform_;
	keyTutorialModel_.Update();
	buttonTutorialModel_.Update();
}

void KeyTutorial::Draw(Camera* camera) {
	if (transform_.scale.Length() <= 0.1f) {
		return;
	}

	if (engineCore_->GetXInputController()->GetIsActiveController(0)) {
		buttonTutorialModel_.Draw(camera);
	} else {
		keyTutorialModel_.Draw(camera);
	}
}
