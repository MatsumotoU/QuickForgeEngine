#include "Reticle.h"

void Reticle::Initialize(EngineCore* engineCore) {
	engineCore_ = engineCore;
	model_.Initialize(engineCore_);
	model_.LoadModel("Resources", "Reticle.obj", COORDINATESYSTEM_HAND_RIGHT);
	transform_.translate = { 0.0f, 0.0f, 30.0f };
	isActive_ = true;

	reticleGH_ = engineCore_->GetTextureManager()->LoadTexture("Resources/uvChecker.png");

	sprite_.Initialize(
		engineCore_->GetDirectXCommon(), engineCore_->GetTextureManager(), engineCore_->GetImGuiManager(),
		100.0f, 100.0f, engineCore_->GetGraphicsCommon()->GetTrianglePso(kBlendModeNormal));
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
	model_.worldMatrix_ = Matrix4x4::Multiply(model_.worldMatrix_, player_->GetWorldMatrix());
}

void Reticle::Draw(Camera* camera) {
	if (!isActive_) {
		return;
	}
	model_.Draw(camera);
}

void Reticle::SetPlayer(Player* player) {
	player_ = player;
}
