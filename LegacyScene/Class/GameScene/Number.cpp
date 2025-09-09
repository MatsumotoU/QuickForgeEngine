#include "Number.h"
#include <algorithm>
#include "Utility/MyEasing.h"

void Number::Initialize(EngineCore* engineCore, Camera* camera) {
	for (uint32_t i = 0; i < 10; ++i) {
		numberModel_[i] = std::make_unique<Model>(engineCore, camera);
		numberModel_[i]->LoadModel("Resources/Model/UI", std::to_string(i) + ".obj", COORDINATESYSTEM_HAND_RIGHT);
		numberModel_[i]->transform_.scale = { 1.0f,1.0f,1.0f };
		numberModel_[i]->transform_.rotate.x = 3.14f * 0.5f;
		numberModel_[i]->transform_.translate = { 0.0f,5.0f,0.0f };
	}

	transform_.translate = { -1.8f,4.0f,3.9f };
	transform_.rotate = { 1.9f,0.4f,-3.1f };
	transform_.scale = { 1.0f,1.0f,1.0f };

	color_ = { 1.0f,1.0f,1.0f,1.0f };
	engineCore_ = engineCore;
	timer_ = 0.0f;
}

void Number::Update() {
	timer_ += engineCore_->GetDeltaTime();

	MyEasing::SimpleEaseIn(&transform_.scale.x, 1.0f, 0.3f);
	MyEasing::SimpleEaseIn(&transform_.scale.y, 1.0f, 0.3f);
	MyEasing::SimpleEaseIn(&transform_.scale.z, 1.0f, 0.3f);

	for (uint32_t i = 0; i < 10; ++i) {
		numberModel_[i]->transform_ = transform_;
		numberModel_[i]->SetColor(color_);
		numberModel_[i]->Update();
	}

	if (timer_ > 0.8f) {
		transform_.scale.x = 1.3f;
		transform_.scale.y = 1.3f;
		transform_.scale.z = 1.3f;
		timer_ = 0.0f;
	}
}

void Number::Draw() {
	if (number_ < numberModel_.size()) {
		numberModel_[number_]->Draw();
	}
}

void Number::SetNumber(uint32_t number) {
	number = std::clamp(number, 0u, 9u);
	if (number_ != number) {
		number_ = number;
	}
}
