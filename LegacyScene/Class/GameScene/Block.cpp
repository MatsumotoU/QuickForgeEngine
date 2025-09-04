#include "Block.h"

void Block::Initialize(EngineCore* engineCore, Camera* camera) {
	engineCore_ = engineCore;
	camera_ = camera;

	model_ = std::make_unique<Model>(engineCore_, camera_);
	model_->LoadModel("Resources", "Cube.obj", COORDINATESYSTEM_HAND_LEFT);
}

void Block::Update() {
	model_->Update();
}

void Block::Draw() {
	model_->Draw();
}
