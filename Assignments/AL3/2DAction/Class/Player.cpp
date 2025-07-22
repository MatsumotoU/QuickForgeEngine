#include "Player.h"
#include <algorithm>
#include "../../../../Engine/Math/MyMath.h"
#include "MapChip.h"

void Player::Initialize(EngineCore* engineCore) {
	engineCore_ = engineCore;
	model_.Initialize(engineCore_);
	model_.LoadModel("Resources", "Speaker.obj", COORDINATESYSTEM_HAND_RIGHT);

	turnTime_ = 0.0f;
	turnTable[0] = 3.14f / 2.0f + 3.14f;
	turnTable[1] = 3.14f / 2.0f;
	lrDirection_ = LRDirection::kRight;

	transform_.scale.x = 2.0f;
	transform_.scale.y = 2.0f;
	transform_.scale.z = 2.0f;
	transform_.rotate.y = 3.14f / 2.0f;

	velocity_ = {};
	acceleration_ = {};

	isGround_ = true;
	isLanding_ = false;

	map_ = nullptr;
	isActive_ = true;

	isAttackCharge_ = false;
	isAttacking_ = false;
	attackChargeTime_ = 0.0f;

	bahaviorType_ = BahaviorType::kIdle;

	billboard_.Initialize(engineCore_,1.0f,1.0f);
}

void Player::Update() {
	if (!isActive_) {
		return;
	}

	// マップの位置取得
	uint32_t xIndex = static_cast<uint32_t>(transform_.translate.x / kBlockWidth);
	uint32_t yIndex = kNumBlockVertical - static_cast<uint32_t>(transform_.translate.y / kBlockHeight) - 1;
	xIndex;
	yIndex;

	/*if (!isGround_) {
		if (transform_.translate.y < 3.0f) {
			isGround_ = true;
			isLanding_ = false;
			acceleration_.y = 0.0f;
			velocity_.y = 0.0f;
			transform_.translate.y = 3.0f;
		}
	}*/

	if (map_->GetMapChipTypeByIndex(xIndex, yIndex + 1) == MapChipType::kBlank) {
		isGround_ = false;
		isLanding_ = true;
	}

	if (!isGround_) {
		if (map_->GetMapChipTypeByIndex(xIndex, yIndex + 1) == MapChipType::kBlock) {
			if ((map_->GetMapChipPositionByIndex(xIndex, yIndex + 1) - transform_.translate).Length() <= kBlockWidth) {
				isGround_ = true;
				isLanding_ = false;
				acceleration_.y = 0.0f;
				velocity_.y = 0.0f;

				transform_.translate.y = map_->GetMapChipPositionByIndex(xIndex, yIndex + 1).y + kBlockHeight;
			}
		}
	}

	switch (bahaviorType_)
	{
	case BahaviorType::kIdle:
		Move();
		break;
	case BahaviorType::kMove:
		Move();
		break;
	case BahaviorType::kAttack:
		Attack();
		break;
	default:
		break;
	}
	
	acceleration_.y *= 0.9f;
	acceleration_.x *= 0.9f;
	
	acceleration_.x = std::clamp(acceleration_.x, -kMaxAcceleration, kMaxAcceleration);

	velocity_.x += acceleration_.x;
	velocity_.y += acceleration_.y;

	velocity_.x *= 0.98f;
	velocity_.y *= 0.98f;

	if (bahaviorType_ != BahaviorType::kAttack) {
		velocity_.x = std::clamp(velocity_.x, -kMaxVelocity, kMaxVelocity);
	} 

	transform_.translate.x += velocity_.x * engineCore_->GetDeltaTime();
	transform_.translate.y += velocity_.y * engineCore_->GetDeltaTime();

	// 回転処理
	if (turnTime_ > 0.0f) {
		turnTime_ -= engineCore_->GetDeltaTime();
		float t = turnTime_ / kTimeTurn;
		transform_.rotate.y = MyMath::Leap(nowTurnY_, turnTable[static_cast<uint32_t>(lrDirection_)], t);
	}

	if (map_->GetMapChipTypeByIndex(xIndex - 1, yIndex) == MapChipType::kBlock) {
		if ((map_->GetMapChipPositionByIndex(xIndex - 1, yIndex) - transform_.translate).Length() <= kWith * 0.5f + kBlockWidth * 0.5f) {
			transform_.translate.x -= velocity_.x * engineCore_->GetDeltaTime();
		}
	}

	if (map_->GetMapChipTypeByIndex(xIndex + 1, yIndex) == MapChipType::kBlock) {
		if ((map_->GetMapChipPositionByIndex(xIndex + 1, yIndex) - transform_.translate).Length() <= kWith * 0.5f + kBlockWidth * 0.5f) {
			transform_.translate.x -= velocity_.x * engineCore_->GetDeltaTime();
		}
	}

	if (map_->GetMapChipTypeByIndex(xIndex, yIndex - 1) == MapChipType::kBlock) {
		if ((map_->GetMapChipPositionByIndex(xIndex, yIndex - 1) - transform_.translate).Length() <= kWith * 0.5f + kBlockWidth * 0.5f) {
			transform_.translate.y -= velocity_.y * engineCore_->GetDeltaTime();
			acceleration_.y = 0.0f;
			velocity_.y = 0.0f;
		}
	}
	
	ImGui::Text("center %f", (map_->GetMapChipPositionByIndex(xIndex, yIndex) - transform_.translate).Length());
	ImGui::Text("right %f", (map_->GetMapChipPositionByIndex(xIndex + 1, yIndex) - transform_.translate).Length());
	ImGui::Text("left %f", (map_->GetMapChipPositionByIndex(xIndex - 1, yIndex) - transform_.translate).Length());

	
	model_.transform_ = transform_;
	model_.Update();
}

void Player::Draw(Camera* camera) {
	if (!isActive_) {
		return;
	}

	if (isAttacking_) {
		billboard_.Update(camera->transform_.rotate);
		billboard_.Draw(0, camera);
	}

	ImGui::Begin("player");
	ImGui::DragFloat3("transition", &transform_.translate.x);
	ImGui::DragFloat3("velocity", &velocity_.x);
	ImGui::DragFloat3("acceleration", &acceleration_.x);
	uint32_t xIndex = static_cast<uint32_t>(transform_.translate.x / kBlockWidth);
	uint32_t yIndex = kNumBlockVertical - static_cast<uint32_t>(transform_.translate.y / kBlockHeight) - 1;
	ImGui::Text("xIndex:%d yIndex:%d",xIndex, yIndex);
	ImGui::Text("%f %f", map_->GetMapChipPositionByIndex(xIndex, yIndex).x, map_->GetMapChipPositionByIndex(xIndex, yIndex).y);
	ImGui::Text("Deltatime:%f", engineCore_->GetDeltaTime());
	ImGui::End();

	model_.Draw(camera);
}

void Player::Move() {
	DirectInputManager* input = engineCore_->GetInputManager();
	if (input->keyboard_.GetPress(DIK_RIGHT)) {
		acceleration_.x += kAcceleration * engineCore_->GetDeltaTime();

		if (lrDirection_ != LRDirection::kRight) {
			turnTime_ = kTimeTurn;
			nowTurnY_ = transform_.rotate.y;
			lrDirection_ = LRDirection::kRight;
		}
	}
	if (input->keyboard_.GetPress(DIK_LEFT)) {
		acceleration_.x -= kAcceleration * engineCore_->GetDeltaTime();

		if (lrDirection_ != LRDirection::kLeft) {
			turnTime_ = kTimeTurn;
			nowTurnY_ = transform_.rotate.y;
			lrDirection_ = LRDirection::kLeft;
		}
	}
	if (input->keyboard_.GetPress(DIK_UP)) {
		if (!isLanding_) {
			isLanding_ = true;
			isGround_ = false;
			velocity_.y += kJumpPower;
		}
	}

	if (input->keyboard_.GetPress(DIK_SPACE)) {
		isAttackCharge_ = true;
		bahaviorType_ = BahaviorType::kAttack;
	}

	if (!isGround_) {
		acceleration_.y -= 9.8f * engineCore_->GetDeltaTime();
	}
}

void Player::Attack() {
	if (isAttackCharge_) {
		velocity_.x = 0.0f;
		velocity_.y = 0.0f;
		acceleration_.x = 0.0f;
		acceleration_.y = 0.0f;

		attackChargeTime_ += engineCore_->GetDeltaTime();

		float attackCargeDir = 1.0f;
		if (lrDirection_ != LRDirection::kLeft) {
			attackCargeDir = -1.0f;
		}

		transform_.rotate.z = attackChargeTime_ * -attackCargeDir;
		transform_.translate.x += engineCore_->GetDeltaTime() * attackCargeDir;
		billboard_.transform_.translate = { transform_.translate.x + 1.5f * -attackCargeDir,transform_.translate.y ,transform_.translate.z };
		

		if (attackChargeTime_ >= 0.3f) {
			transform_.rotate.z = 0.0f;
			attackChargeTime_ = 0.0f;
			isAttackCharge_ = false;
			transform_.scale.x = 2.0f;
			velocity_.x = 20.0f * -attackCargeDir;
			isAttacking_ = true;
		}
		return;
	} else {

		if (velocity_.Length() <= 15.0f) {
			isAttacking_ = false;
			bahaviorType_ = BahaviorType::kMove;
		}
	}

	float attackCargeDir = 1.0f;
	if (lrDirection_ != LRDirection::kLeft) {
		attackCargeDir = -1.0f;
	}
	billboard_.transform_.translate = { transform_.translate.x + 1.5f * -attackCargeDir,transform_.translate.y ,transform_.translate.z };

}

bool Player::GetIsActive() {
	return isActive_;
}

bool Player::GetIsAttacking() {
	return isAttacking_;
}

void Player::SetIsActive(bool isActive) {
	isActive_ = isActive;
}

void Player::SetMap(MapChip* map) {
	map_ = map;
}
