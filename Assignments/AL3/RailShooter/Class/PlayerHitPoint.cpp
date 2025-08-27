#include "PlayerHitPoint.h"
#include "Utility/MyEasing.h"

void PlayerHitPoint::Initialize(EngineCore* engineCore) {
	for (int i = 0; i < 3; i++) {
		model_[i].Initialize(engineCore);
		model_[i].LoadModel("Resources", "Azarasi.obj", COORDINATESYSTEM_HAND_RIGHT);
		model_[i].transform_.scale = { 0.3f,0.3f,0.3f };
		model_[i].transform_.rotate = { 3.14f*0.5f, 0.0f, 0.0f };
	}
	pos_ = { 1.7f, -0.77f, 5.0f };
	hitp_ = 3;
}

void PlayerHitPoint::Update() {
	for (int i = 0; i < 3; i++) {
		if (i >= hitp_) {
			model_[i].transform_.rotate.y += 1.0f;
			Eas::SimpleEaseIn(&model_[i].transform_.scale.x, 0.0f, 0.3f);
			Eas::SimpleEaseIn(&model_[i].transform_.scale.y, 0.0f, 0.3f);
			Eas::SimpleEaseIn(&model_[i].transform_.scale.z, 0.0f, 0.3f);
		}
		model_[i].transform_.rotate.y += 0.01f;
		model_[i].transform_.translate.x = -static_cast<float>(i)*0.3f + pos_.x;
		model_[i].transform_.translate.y = pos_.y;
		model_[i].transform_.translate.z = pos_.z;
		model_[i].Update();
	}
}

void PlayerHitPoint::Draw(Camera* camera) {
	for (int i = 0; i < 3; i++) {
		if (model_[i].transform_.scale.Length() > 0.01f) {
			model_[i].worldMatrix_ = Matrix4x4::Multiply(model_[i].worldMatrix_, camera->GetWorldMatrix());
			model_[i].Draw(camera);
		}
	}
}

void PlayerHitPoint::SetHitPoint(int p) {
	hitp_ = p;
}
