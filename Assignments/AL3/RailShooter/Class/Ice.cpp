#include "Ice.h"

void Ice::Initialize(EngineCore* engineCore) {
	engineCore_ = engineCore;
	iceModel_.Initialize(engineCore_);
	iceModel_.LoadModel("Resources", "Ice.obj", COORDINATESYSTEM_HAND_RIGHT);
	syropeModel_.Initialize(engineCore_);
	syropeModel_.LoadModel("Resources", "Syrup.obj", COORDINATESYSTEM_HAND_RIGHT);

	syropeColor_ = { 1.0f,1.0f,1.0f,1.0f };
}

void Ice::Update() {
	iceModel_.transform_ = transform_;
	syropeModel_.transform_ = transform_;

	syropeModel_.material_.materialData_->color = syropeColor_;

	iceModel_.Update();
	syropeModel_.Update();
}

void Ice::Draw(Camera* camera) {
	iceModel_.Draw(camera);
	syropeModel_.Draw(camera);
}
