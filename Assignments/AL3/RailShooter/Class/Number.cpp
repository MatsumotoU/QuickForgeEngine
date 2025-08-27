#include "Number.h"

void Number::Initialize(EngineCore* engineCore) {
	engineCore_ = engineCore;
	value_ = 0;

	for (int i = 0; i < 10; i++) {
		models_[i].Initialize(engineCore_);
		models_[i].LoadModel("Resources/", std::to_string(i) + ".obj", COORDINATESYSTEM_HAND_RIGHT);
	}
}

void Number::Update() {
	models_[value_].transform_ = transform_;
	models_[value_].Update();
}

void Number::Draw(Camera* camera) {
	models_[value_].Draw(camera);
}

Model* Number::GetCerrentModel() {
	return &models_[value_];
}
