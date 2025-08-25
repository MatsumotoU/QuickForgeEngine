#include "Ground.h"

void Ground::Init(EngineCore* engineCore) {
	engineCore_ = engineCore;
	groundModel_.Initialize(engineCore_);
	groundModel_.LoadModel("Resources", "19_snow_Home.obj", COORDINATESYSTEM_HAND_RIGHT);

}

void Ground::Update() {

	groundModel_.Update();
}

void Ground::Draw(Camera* camera) {
	groundModel_.Draw(camera);
}
