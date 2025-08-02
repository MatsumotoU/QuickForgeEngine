#include "Player.h"
#include <algorithm>
#include "../../../../Engine/Math/MyMath.h"
#include "Utility/MyEasing.h"

void Player::Initialize(EngineCore* engineCore) {
	engineCore_ = engineCore;
	model_.Initialize(engineCore_);
	model_.LoadModel("Resources", "plot.obj", COORDINATESYSTEM_HAND_RIGHT);

	shieldModel_.Initialize(engineCore_);
	shieldModel_.LoadModel("Resources", "Shield.obj", COORDINATESYSTEM_HAND_RIGHT);
	shieldModel_.material_.materialData_->enableLighting = false;
	shieldModel_.material_.materialData_->color = { 0.4f, 0.4f, 1.0f, 1.0f };

	moveSpeed_ = 30.0f;

	maxShotCooldown_ = 3.0f;
	shotCooldown_ = 0.0f;

	blendNum_ = 0;

	isShield_ = false;
	isBreaking_ = false;
	isActive_ = true;
	parentMatrix_ = Matrix4x4::MakeIndentity4x4();

	transform_.translate.z = 8.0f;

	sheildPoint_ = 0.0f;
	hitPoint_ = 100;
	frameCount_ = 0;

	// モーション登録
	motionState_ = NORMAL;
	motionFuncMap_[NORMAL] = std::bind(&Player::NormalMotion, this);
	motionFuncMap_[DAMAGE] = std::bind(&Player::DamageMotion, this);
	shieldLevel_ = 0;
}

void Player::Update() {
	if (!isActive_) {
		return;
	}
	frameCount_++;
	motionTime_ -= engineCore_->GetDeltaTime();
	motionFuncMap_[motionState_]();

	DirectInputManager* input = engineCore_->GetInputManager();

	Vector2 moveDir = (input->GetKeyMoveDir() + engineCore_->GetXInputController()->GetLeftStick(0)).Normalize();
	
	acceleration_.x = std::clamp(acceleration_.x, -kLimitSpeed, kLimitSpeed);
	acceleration_.y = std::clamp(acceleration_.y, -kLimitSpeed, kLimitSpeed);

	velocity_ += acceleration_ * engineCore_->GetDeltaTime();
	velocity_.x += moveDir.x * moveSpeed_ * engineCore_->GetDeltaTime();
	velocity_.y += moveDir.y * moveSpeed_ * engineCore_->GetDeltaTime();
	acceleration_ = acceleration_ * 0.91f;

	velocity_.x = std::clamp(velocity_.x, -kLimitSpeed, kLimitSpeed);
	velocity_.y = std::clamp(velocity_.y, -kLimitSpeed, kLimitSpeed);

	transform_.translate += velocity_ * engineCore_->GetDeltaTime();
	velocity_ = velocity_ * 0.93f;

	transform_.translate.x = std::clamp(transform_.translate.x, -kLimitMoveWidh, kLimitMoveWidh);
	transform_.translate.y = std::clamp(transform_.translate.y, -kLimitMoveHeight, kLimitMoveHeight);

	if (moveDir.Length() > 0.01f) {
		Eas::SimpleEaseIn(&transform_.rotate.z, -moveDir.x * 0.3f, 0.1f);
	} else {
		Eas::SimpleEaseIn(&transform_.rotate.z, velocity_.x * 0.2f, 0.05f);
	}
	

	isShield_ = false;
	moveSpeed_ = 30.0f;
	if (input->keyboard_.GetPress(DIK_SPACE) || engineCore_->GetXInputController()->GetPressButton(XINPUT_GAMEPAD_A, 0)) {
		isShield_ = true;
		if (!isBreaking_) {
			moveSpeed_ = 15.0f; // シールド中は移動速度を落とす
		}
	} else {
		if (shieldLevel_ > 0) {
			sheildPoint_ = 0.0f;
			isRevenge_ = true;
		}
	}

	// si-ールドの状態更新
	if (isShield_ && !isBreaking_) {
		Eas::SimpleEaseIn(&shieldModel_.transform_.scale.x, 1.0f, 0.5f);
		Eas::SimpleEaseIn(&shieldModel_.transform_.scale.y, 1.0f, 0.5f);
		Eas::SimpleEaseIn(&shieldModel_.transform_.scale.z, 1.0f, 0.5f);

		float shieldPointRatio = sheildPoint_ / 100.0f;
		shieldModel_.material_.materialData_->color = 
			Vector4::Leap({ 0.3f,0.3f,1.0f,1.0f }, { 0.2f,0.0f,0.0f,1.0f }, shieldPointRatio);

		shieldModel_.transform_.rotate.x += 
			sinf(static_cast<float>(frameCount_) * engineCore_->GetDeltaTime() * 0.01f ) * (shieldPointRatio + 0.1f);
		shieldModel_.transform_.rotate.y += 
			cosf(static_cast<float>(frameCount_) * engineCore_->GetDeltaTime() * 0.01f ) * (shieldPointRatio + 0.1f);
		shieldModel_.transform_.rotate.z += 
			sinf(static_cast<float>(frameCount_) * engineCore_->GetDeltaTime() * 0.01f ) * (shieldPointRatio + 0.1f);

	} else {
		Eas::SimpleEaseIn(&shieldModel_.transform_.scale.x, 0.0f, 0.2f);
		Eas::SimpleEaseIn(&shieldModel_.transform_.scale.y, 0.0f, 0.2f);
		Eas::SimpleEaseIn(&shieldModel_.transform_.scale.z, 0.0f, 0.2f);
	}

	if (!isShield_) {
		isBreaking_ = false;
		if (sheildPoint_ > 0.0f) {
			sheildPoint_ -= engineCore_->GetDeltaTime() * 2.0f;
			if (sheildPoint_ < 0.0f) {
				sheildPoint_ = 0.0f;
			}
		}

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

	/*if (input->keyboard_.GetPress(DIK_D)) {
		transform_.rotate.y += 0.05f;
	}
	if (input->keyboard_.GetPress(DIK_A)) {
		transform_.rotate.y -= 0.05f;
	}
	if (input->keyboard_.GetPress(DIK_W)) {
		transform_.rotate.x += 0.05f;
	}
	if (input->keyboard_.GetPress(DIK_S)) {
		transform_.rotate.x -= 0.05f;
	}*/

	model_.transform_ = transform_;

	model_.Update();
	model_.worldMatrix_ = Matrix4x4::Multiply(model_.worldMatrix_, parentMatrix_);

	shieldModel_.transform_.translate = transform_.translate;
	shieldModel_.Update();
	shieldModel_.worldMatrix_ = Matrix4x4::Multiply(shieldModel_.worldMatrix_, parentMatrix_);

	shieldLevel_ = static_cast<int>(sheildPoint_ / 33.3f);
}

void Player::Draw(Camera* camera) {
	if (!isActive_) {
		return;
	}
	model_.Draw(camera);
	if (shieldModel_.transform_.scale.Length() >= 0.1f) {
		shieldModel_.Draw(camera);
	}

	ImGui::Begin("Player");
	ImGui::DragFloat3("ModelTransform", &model_.transform_.translate.x, 0.1f);
	ImGui::DragFloat3("ModelRotate", &model_.transform_.rotate.x, 0.1f);
	ImGui::DragFloat3("translate", &transform_.translate.x);
	ImGui::DragFloat3("ModelRotate", &transform_.rotate.x, 0.1f);
	ImGui::DragFloat3("velocity", &velocity_.x);
	ImGui::DragFloat3("acceleration", &acceleration_.x);
	ImGui::DragFloat4("color", &model_.material_.materialData_->color.x, 0.1f);
	ImGui::DragInt("BlendMode", &blendNum_);
	ImGui::Text("worldPos %f %f %f", GetWorldPosition().x, GetWorldPosition().y, GetWorldPosition().z);

	if (ImGui::Button("SetBlendMode")) {
		model_.SetBlendmode(static_cast<BlendMode>(blendNum_));
	}
	ImGui::End();
}

void Player::OnCollision(const nlohmann::json& otherData) {
	if (!otherData.contains("Attack")) {
		return;
	}
	if (motionState_ == DAMAGE) {
		// 既にダメージモーション中は無視
		return;
	}

	// 被弾時のモーション
	if (isShield_ && !isBreaking_) {
		float attack = otherData["Attack"].get<float>();
		sheildPoint_ += attack;
		shieldModel_.transform_.scale = { 1.3f,1.5f,1.3f };
		if (sheildPoint_ >= 100.0f) {
			sheildPoint_ = 100.0f;
			isBreaking_ = true;
			shieldModel_.transform_.scale = { 2.0f,4.0f,2.0f };
		}

	} else {
		hitPoint_ -= otherData["Attack"].get<int>();
		motionState_ = DAMAGE;
		motionTime_ = 1.0f;
	}

	// 被弾時の吹っ飛び
	if (!otherData.contains("Position")) {
		return;
	}
	Vector3 hitPos = {
		otherData["Position"]["x"].get<float>(),
		otherData["Position"]["y"].get<float>(),
		otherData["Position"]["z"].get<float>()
	};
	if (isShield_) {
		// シールドがある場合は吹っ飛ばない
		return;
	}

	velocity_ = (transform_.translate - hitPos).Normalize() * 100.0f;
	if (velocity_.z > 0.0f) {
		Vector3 zforce = transform_.translate.Normalize() * velocity_.z;
		velocity_ += zforce;
	}
	velocity_.z = 0.0f; // z軸の吹っ飛びは無効化
}

bool Player::GetIsActive() {
	return isActive_;
}

bool Player::GetIsShot() {
	return isShot_;
}

bool Player::GetIsShield() {
	return isShield_;
}

Matrix4x4 Player::GetParentMatrix() {
	return parentMatrix_;
}

Matrix4x4 Player::GetParentWorldMatrix() {
	Matrix4x4 result = Matrix4x4::MakeIndentity4x4();
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

Vector3 Player::GetDir() {
	return Vector3::Transform({ 0.0f,0.0f,1.0f }, Matrix4x4::MakeRotateXYZMatrix(model_.transform_.rotate));
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

void Player::NormalMotion() {
	Eas::SimpleEaseIn(&transform_.scale.x, 1.0f, 0.1f);
	Eas::SimpleEaseIn(&transform_.scale.y, 1.0f, 0.1f);
	Eas::SimpleEaseIn(&transform_.scale.z, 1.0f, 0.1f);

	Eas::SimpleEaseIn(&model_.material_.materialData_->color.x, 1.0f, 0.1f);
	Eas::SimpleEaseIn(&model_.material_.materialData_->color.y, 1.0f, 0.1f);
	Eas::SimpleEaseIn(&model_.material_.materialData_->color.z, 1.0f, 0.1f);
	Eas::SimpleEaseIn(&model_.material_.materialData_->color.w, 1.0f, 0.1f);
	motionTime_ = 0.0f;
}

void Player::DamageMotion() {
	if (frameCount_ % 2) {
		model_.material_.materialData_->color.x = 1.0f;
		model_.material_.materialData_->color.y = 0.0f;
		model_.material_.materialData_->color.z = 0.0f;
	} else {
		model_.material_.materialData_->color.x = 1.0f;
		model_.material_.materialData_->color.y = 1.0f;
		model_.material_.materialData_->color.z = 1.0f;
	}
	transform_.scale.x = sinf(static_cast<float>(frameCount_ / 3)) * 0.3f + 0.7f;
	transform_.scale.y = cosf(static_cast<float>(frameCount_ / 3)) * 0.3f + 0.7f;
	transform_.scale.z = sinf(static_cast<float>(frameCount_ / 3)) * 0.3f + 0.7f;

	if (motionTime_ < 0.0f) {
		motionState_ = NORMAL;
	}
}
