#include "Reticle.h"

void Reticle::Initialize(EngineCore* engineCore) {
	engineCore_ = engineCore;
	model_.Initialize(engineCore_);
	model_.LoadModel("Resources", "Reticle.obj", COORDINATESYSTEM_HAND_RIGHT);
	transform_.translate = { 0.0f, 0.0f, 30.0f };
	isActive_ = true;

	reticleGH_ = engineCore_->GetTextureManager()->LoadTexture("Resources/Reticle.png");

	sprite_.Initialize(
		engineCore_,100.0f,100.0f);
}

void Reticle::Update() {
	if (engineCore_->GetInputManager()->keyboard_.GetPress(DIK_SPACE)) {
		transform_.rotate.z += 3.14f * engineCore_->GetDeltaTime();
	}
	transform_.rotate.z += 3.14f * engineCore_->GetDeltaTime();

	if (!isActive_) {
		return;
	}
	model_.transform_ = transform_;
	model_.Update();
	model_.worldMatrix_ = Matrix4x4::Multiply(Matrix4x4::MakeAffineMatrix(transform_.scale,transform_.rotate,transform_.translate), player_->GetWorldMatrix());

	
}

void Reticle::Draw(Camera* camera) {
	if (!isActive_) {
		return;
	}
	model_.Draw(camera);
    //spriteTransform_.translate = Vector3::WorldToScreen(transform_.translate, model_.worldMatrix_, camera, 1280.0f, 720.0f);
	spriteTransform_.translate = camera->GetScreenPos(Vector3::Zero(), model_.worldMatrix_);
    spriteTransform_.translate.x -= 50.0f;
    spriteTransform_.translate.y -= 50.0f;
	sprite_.DrawSprite(spriteTransform_, reticleGH_, camera);
}

void Reticle::SetPlayer(Player* player) {
	player_ = player;
}

Vector3 Reticle::GetWorldPos() {
	return Vector3::Transform(Vector3::Zero(),Matrix4x4::MakeAffineMatrix(model_.transform_.scale, model_.transform_.rotate, model_.transform_.translate));
}
