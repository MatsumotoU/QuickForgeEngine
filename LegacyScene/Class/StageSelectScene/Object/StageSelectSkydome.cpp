#include "StageSelectSkydome.h"
#include "../Engine/Model/Model.h"
#include <numbers>

void StageSelectSkydome::Initialize(Model *model) {
	assert(model);
	model_ = model;
	model_->SetDirectionalLightDir({ 0.0f, 0.0f, 0.0f });
	model_->SetDirectionalLightIntensity(2.5f);
}

void StageSelectSkydome::Update() {
	model_->Update();
}

void StageSelectSkydome::Draw() {
	model_->Draw();
}
