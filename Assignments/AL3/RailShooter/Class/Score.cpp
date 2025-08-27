#include "Score.h"
#include "Utility/MyEasing.h"

void Score::Initialize(EngineCore* engineCore) {
	engineCore_ = engineCore;
	score_ = 0;
	position_ = { 0.0f, 0.0f, 0.0f };
	for (int i = 0; i < 8; i++) {
		numbers_[i].Initialize(engineCore_);
	}
}

void Score::Update() {
	int displayScore = score_;
	for (int i = 0; i < 8; i++) {
		Eas::SimpleEaseIn(&numbers_[7 - i].transform_.scale.x, 1.0f, 0.1f);
		Eas::SimpleEaseIn(&numbers_[7 - i].transform_.scale.y, 1.0f, 0.1f);
		Eas::SimpleEaseIn(&numbers_[7 - i].transform_.scale.z, 1.0f, 0.1f);
		numbers_[7 - i].transform_.rotate = { 0.0f,3.14f,0.0f };
		numbers_[7 - i].transform_.translate = position_ + Vector3{ -0.15f * i, 0.0f, 0.0f };
		numbers_[7 - i].value_ = displayScore % 10;
		if (oldNumbers_[i] != numbers_[7 - i].value_) {
			numbers_[7 - i].transform_.scale = { 1.5f,1.5f,1.5f };
			numbers_[7 - i].transform_.rotate = { 0.0f,3.14f + 3.14f / 2.0f,0.0f };
			oldNumbers_[i] = displayScore % 10;
		}
		displayScore /= 10;
		numbers_[7 - i].Update();
	}
}

void Score::Draw(Camera* camera) {
	for (int i = 0; i < 8; i++) {
		numbers_[7 - i].GetCerrentModel()->worldMatrix_ = Matrix4x4::Multiply(numbers_[7 - i].GetCerrentModel()->worldMatrix_, camera->GetWorldMatrix());
		numbers_[7 - i].Draw(camera);
	}
}

void Score::AddScore(int score) {
	score_ += score;	
}
