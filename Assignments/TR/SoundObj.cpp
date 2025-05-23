#include "SoundObj.h"

void SoundObj::Initialize(EngineCore* engineCore) {
	engineCore_ = engineCore;
	model_.Initialize(engineCore_);
	model_.LoadModel("Resources", "Speaker.obj", COORDINATESYSTEM_HAND_RIGHT);

	transform_.rotate.y = 3.14f;

	lookTargetPosition_ = nullptr;
}

void SoundObj::Update() {
	if (lookTargetPosition_ != nullptr) {
		transform_.rotate = Vector3::LookAt(transform_.translate, *lookTargetPosition_);
		transform_.rotate.y += 3.14f;
	}
}

void SoundObj::Draw(Camera* camera) {
	model_.Draw(transform_, camera);
}

void SoundObj::SetTarget(Vector3* target) {
	lookTargetPosition_ = target;
}
