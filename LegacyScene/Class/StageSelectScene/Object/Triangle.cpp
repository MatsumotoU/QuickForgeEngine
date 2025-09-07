#include "Triangle.h"
#include "../Engine/Model/Model.h"
#include "../Engine/Input/DirectInput/DirectInputManager.h"
#include "../Engine/Utility/MyEasing.h"
#include "../State/TriangleState.h"
#include <numbers>

Triangle::~Triangle() {
	delete currentState_;
}

void Triangle::Initialize(Model *model, DirectInputManager *input, Direction direction) {
	// モデルを設定
	assert(model);
	model_ = model;

	// 入力を設定
	assert(input);
	input_ = input;

	// 方向を設定
	direction_ = direction;

	// X座標テーブル
	std::array<float, 2> destinationPositionXTable = {
		-1.5f,	// kLeft
		1.5f,	// kRight
	};

	// Z回転角テーブル
	std::array<float, 2> destinationRotationZTable = {
		std::numbers::pi_v<float> / 2.0f,	// kLeft
		-std::numbers::pi_v<float> / 2.0f	// kRight
	};

	// 座標と回転角を設定
	model_->transform_.translate.x = destinationPositionXTable[static_cast<uint32_t>(direction_)];
	model_->transform_.rotate.z = destinationRotationZTable[static_cast<uint32_t>(direction_)];

	// 現在の状態を設定
	currentState_ = new TriangleStateIdle(this);
	currentState_->Initialize();
}

void Triangle::Update() {
	// Y回転角テーブル
	std::array<float, 2> destinationRotationYTable = {
		-0.02f,	// kLeft
		0.02f	// kRight
	};

	// Y回転角を更新
	model_->transform_.rotate.y += destinationRotationYTable[static_cast<uint32_t>(direction_)];

	// モデルを更新
	model_->Update();

	// 親のワールド行列を掛け合わせる
	model_->worldMatrix_ = model_->worldMatrix_ * parentWorldMatrix_;
}

void Triangle::Draw() {
	model_->Draw();
}

void Triangle::ChangeState(BaseTriangleState *newState) {
	delete currentState_;
	currentState_ = newState;
	currentState_->Initialize();
}

void Triangle::StateUpdate() { currentState_->Update(); }

void Triangle::Shrink() {
	easeTimer_++;
	model_->transform_.scale.x = MyEasing::EaseIn(easeTimer_ / kEaseDuration, 2.0f, largeScale.x, smallScale.x);
	model_->transform_.scale.y = MyEasing::EaseIn(easeTimer_ / kEaseDuration, 2.0f, largeScale.y, smallScale.y);
	model_->transform_.scale.z = MyEasing::EaseIn(easeTimer_ / kEaseDuration, 2.0f, largeScale.z, smallScale.z);
}

void Triangle::Expand() {
	easeTimer_++;
	model_->transform_.scale.x = MyEasing::EaseOut(easeTimer_ / kEaseDuration, 2.0f, smallScale.x, largeScale.x);
	model_->transform_.scale.y = MyEasing::EaseOut(easeTimer_ / kEaseDuration, 2.0f, smallScale.y, largeScale.y);
	model_->transform_.scale.z = MyEasing::EaseOut(easeTimer_ / kEaseDuration, 2.0f, smallScale.z, largeScale.z);
}