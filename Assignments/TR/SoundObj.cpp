#include "SoundObj.h"

void SoundObj::Initialize(EngineCore* engineCore) {
	engineCore_ = engineCore;
	model_.Initialize(engineCore_);
	model_.LoadModel("Resources", "Speaker.obj", COORDINATESYSTEM_HAND_RIGHT);

	transform_.rotate.y = 3.14f;
}

void SoundObj::Update() {
}

void SoundObj::Draw(Camera* camera) {
	model_.Draw(transform_, camera);
}
