#include "Reticle.h"
#include "Utility/MyEasing.h"

void Reticle::Initialize(EngineCore* engineCore) {
	engineCore_ = engineCore;
	model_.Initialize(engineCore_);
	model_.LoadModel("Resources", "1mRing.obj", COORDINATESYSTEM_HAND_RIGHT);
	transform_.translate = { 0.0f, 0.0f, 30.0f };
	isActive_ = true;

	reticleGH_ = engineCore_->GetTextureManager()->LoadTexture("Resources/Reticle.png");

	sprite_.Initialize(
		engineCore_,100.0f,100.0f);

	reticleScale_ = 1.0f;
}

void Reticle::Update() {
	reticleScale_ = static_cast<float>(player_->GetShieldLevel());
	if (player_->GetIsShield()) {
		Eas::SimpleEaseIn(&model_.transform_.scale.x, reticleScale_, 0.5f);
		Eas::SimpleEaseIn(&model_.transform_.scale.y, reticleScale_, 0.5f);
		Eas::SimpleEaseIn(&model_.transform_.scale.z, reticleScale_, 0.5f);
	} else {
		Eas::SimpleEaseIn(&model_.transform_.scale.x, 0.0f, 0.5f);
		Eas::SimpleEaseIn(&model_.transform_.scale.y, 0.0f, 0.5f);
		Eas::SimpleEaseIn(&model_.transform_.scale.z, 0.0f, 0.5f);
	}

	transform_.rotate.z += 3.14f * engineCore_->GetDeltaTime();

	if (!isActive_) {
		return;
	}
	model_.transform_.translate = player_->transform_.translate;
	model_.transform_.translate.x *= -1.0f;
	model_.transform_.translate.y *= -1.0f;

	model_.Update();
	model_.worldMatrix_ = Matrix4x4::MakeAffineMatrix(model_.transform_.scale, model_.transform_.rotate, model_.transform_.translate);
}

void Reticle::Draw(Camera* camera) {
	if (!isActive_) {
		return;
	}

	if (model_.transform_.scale.Length() < 0.1f) {
		return;
	}
	// Draw the reticle model
	model_.Draw(camera);
    //spriteTransform_.translate = Vector3::WorldToScreen(transform_.translate, model_.worldMatrix_, camera, 1280.0f, 720.0f);
	spriteTransform_.translate = camera->GetScreenPos(Vector3::Zero(), model_.worldMatrix_);
    spriteTransform_.translate.x -= 50.0f;
    spriteTransform_.translate.y -= 50.0f;
	//sprite_.DrawSprite(spriteTransform_, reticleGH_, camera);
}

void Reticle::SetPlayer(Player* player) {
	player_ = player;
}

Vector3 Reticle::GetWorldPos() {
	return Vector3::Transform(Vector3::Zero(),Matrix4x4::MakeAffineMatrix(model_.transform_.scale, model_.transform_.rotate, model_.transform_.translate));
}

Vector3 Reticle::GetReticleWorldPos() {
	return Vector3::Transform(Vector3::Zero(),model_.worldMatrix_);
}

Vector3 Reticle::GetTopPos() {
	Vector3 localTop = { 0.0f, model_.transform_.scale.y, 0.0f };
	return Vector3::Transform(localTop, Matrix4x4::MakeAffineMatrix(model_.transform_.scale, model_.transform_.rotate, model_.transform_.translate));
}

Vector3 Reticle::GetTargetWorldPos() {
	return Vector3::Transform(Vector3{0.0f,0.0f,30.0f}, model_.worldMatrix_);;
}
