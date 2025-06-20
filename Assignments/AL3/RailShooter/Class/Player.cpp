#include "Player.h"
#include <algorithm>
#include "../../../../Engine/Math/MyMath.h"

void Player::Initialize(EngineCore* engineCore) {
	engineCore_ = engineCore;
	model_.Initialize(engineCore_);
	model_.LoadModel("Resources", "plot.obj", COORDINATESYSTEM_HAND_RIGHT);

	moveSpeed_ = 50.0f;

	maxShotCooldown_ = 10.0f;
	shotCooldown_ = 0.0f;

	blendNum_ = 0;

	isActive_ = true;
	parentMatrix_ = Matrix4x4::MakeIndentity4x4();

	transform_.translate.z = 30.0f;
}

void Player::Update() {
	if (!isActive_) {
		return;
	}

	DirectInputManager* input = engineCore_->GetInputManager();
	
	Vector2 moveDir = input->GetKeyMoveDir() + engineCore_->GetXInputController()->GetLeftStick(0);

	acceleration_.x += moveDir.x * moveSpeed_ * engineCore_->GetDeltaTime();
	acceleration_.y += moveDir.y * moveSpeed_ * engineCore_->GetDeltaTime();

	acceleration_.x = std::clamp(acceleration_.x, -kLimitSpeed, kLimitSpeed);
	acceleration_.y = std::clamp(acceleration_.y, -kLimitSpeed, kLimitSpeed);

	velocity_ += acceleration_ * engineCore_->GetDeltaTime();
	acceleration_ = acceleration_ * 0.95f;

	velocity_.x = std::clamp(velocity_.x, -kLimitSpeed, kLimitSpeed);
	velocity_.y = std::clamp(velocity_.y, -kLimitSpeed, kLimitSpeed);

	transform_.translate += velocity_ * engineCore_->GetDeltaTime();
	velocity_ = velocity_ * 0.98f;

	transform_.translate.x = std::clamp(transform_.translate.x, -kLimitMoveWidh, kLimitMoveWidh);
	transform_.translate.y = std::clamp(transform_.translate.y, -kLimitMoveHeight, kLimitMoveHeight);

	if (input->keyboard_.GetPress(DIK_SPACE) || engineCore_->GetXInputController()->GetPressButton(XINPUT_GAMEPAD_A,0)) {
		if (shotCooldown_ <= 0.0f) {
			if (!isShot_) {
				isShot_ = true;
				shotCooldown_ = maxShotCooldown_;
			}
		}
		
	}

	if (shotCooldown_ > 0.0f) {
		shotCooldown_--;
	}

	if (input->keyboard_.GetPress(DIK_D)) {
		transform_.rotate.y += 0.05f;
	}
	if (input->keyboard_.GetPress(DIK_A)) {
		transform_.rotate.y -= 0.05f;
	}

	model_.transform_ = transform_;
	
	model_.Update();
	model_.worldMatrix_ = Matrix4x4::Multiply(model_.worldMatrix_, parentMatrix_);
}

void Player::Draw(Camera* camera) {
	if (!isActive_) {
		return;
	}
	model_.Draw(camera);

	ImGui::Begin("Player");
	ImGui::DragFloat3("ModelTransform", &model_.transform_.translate.x, 0.1f);
	ImGui::DragFloat3("ModelRotate", &model_.transform_.rotate.x,0.1f);
	ImGui::DragFloat3("translate", &transform_.translate.x);
	ImGui::DragFloat3("ModelRotate", &transform_.rotate.x, 0.1f);
	ImGui::DragFloat3("velocity", &velocity_.x);
	ImGui::DragFloat3("acceleration", &acceleration_.x);
	ImGui::DragFloat4("color", &model_.material_.materialData_->color.x,0.1f);
	ImGui::DragInt("BlendMode", &blendNum_);
	ImGui::Text("worldPos %f %f %f", GetWorldPosition().x, GetWorldPosition().y, GetWorldPosition().z);

	if (ImGui::Button("SetBlendMode")) {
		model_.SetBlendmode(static_cast<BlendMode>(blendNum_));
	}
	ImGui::End();
}

void Player::OnCollision() {
	//isActive_ = false;
}

bool Player::GetIsActive() {
	return isActive_;
}

bool Player::GetIsShot() {
	return isShot_;
}

Matrix4x4 Player::GetParentMatrix() {
	return parentMatrix_;
}

Matrix4x4 Player::GetParentWorldMatrix() {
	Matrix4x4 result= Matrix4x4::MakeIndentity4x4();
	result = Matrix4x4::Multiply(parentMatrix_, model_.worldMatrix_);
	return result;
}

Matrix4x4 Player::GetWorldMatrix() {
	return model_.worldMatrix_;
}

Matrix4x4 Player::GetRotateMatrix() {
	return Matrix4x4::MakeRotateXYZMatrix(transform_.rotate);
}

Vector3 Player::GetWorldPosition() {
	return Vector3::Transform(Vector3::Zero(), GetParentWorldMatrix());
}

Vector3 Player::GetScreenPosition(Camera* camera) {
	Vector3 result{};
	result = camera->GetScreenPos(Vector3::Zero(), model_.worldMatrix_);
	return result;
}

void Player::SetParent(const Matrix4x4& parentMatrix) {
	parentMatrix_ = parentMatrix;
}

void Player::SetIsActive(bool isActive) {
	isActive_ = isActive;
}

void Player::SetIsShot(bool isShot) {
	isShot_ = isShot;
}
