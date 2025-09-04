#include "Block.h"

void Block::Initialize(EngineCore* engineCore, Camera* camera) {
	engineCore_ = engineCore;
	camera_ = camera;

	model_ = std::make_unique<Model>(engineCore_, camera_);
	model_->LoadModel("Resources", "Cube.obj", COORDINATESYSTEM_HAND_LEFT);

	isDraw_ = true;
}

void Block::Update() {
	model_->Update();
}

void Block::Draw() {
	if (isDraw_) {
		model_->Draw();
	}
}

Transform& Block::GetTransform() {
	return model_->transform_;
}

void Block::SetIsDraw(bool isDraw) {
	isDraw_ = isDraw;
}

void Block::SetColor(const Vector4& color) {
	model_->SetColor(color);
}
