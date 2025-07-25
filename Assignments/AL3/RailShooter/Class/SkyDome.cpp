#include "SkyDome.h"

void SkyDome::Initialize(EngineCore* engineCore) {
	engineCore_ = engineCore;
	model_.Initialize(engineCore);
	model_.LoadModel("Resources", "skyDome.obj", COORDINATESYSTEM_HAND_RIGHT);
	model_.material_.materialData_->enableLighting = false;

	model_.transform_ = transform_;
	model_.Update();
}

void SkyDome::Draw(Camera* camera) {
	model_.Draw(camera);
}
