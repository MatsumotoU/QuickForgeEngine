#include "Enemy.h"
#include "EnemyStateAproach.h"
#include "EnemyStateLeave.h"
#include "Utility/MyEasing.h"

#ifdef _DEBUG
#include "Base/MyDebugLog.h"
#endif // _DEBUG

void Enemy::Initialize(EngineCore* engineCore) {
	engineCore_ = engineCore;
	model_.Initialize(engineCore_);
	model_.LoadModel("Resources", "Triangle.obj", COORDINATESYSTEM_HAND_RIGHT);
	isActive_ = false;

	velocity_ = { 0.0f,0.0f,0.0f };

	transform_.rotate.x = 3.14f * 0.5f;
	transform_.rotate.y = 3.14f;
	leaveSpeed_ = 3.0f;
	phase_ = Phase::Approach;
	moveType_ = MoveType::Normal;

	isShot_ = false;
	shotInterval_ = kMaxShotInterval;
	maxHitPoint_ = 30;
	hitPoint_ = maxHitPoint_;
	motionTime_ = 0.0f;

	ChangeState(std::make_unique<EnemyStateAproach>(this));

	motionState_ = NORMAL;
	motionFuncMap_[NORMAL] = std::bind(&Enemy::NormalMotion, this);
	motionFuncMap_[DAMAGE] = std::bind(&Enemy::DamageMotion, this);
	motionFuncMap_[DEAD] = std::bind(&Enemy::DeadMotion, this);

}

void Enemy::Update() {
	if (!isActive_) {
		return;
	}

	motionFuncMap_[motionState_]();

	frameCount_++;

	state_.get()->Update();

	if (fabsf(transform_.translate.x) >= kXLimit) {
		isActive_ = false;
	}

	if (!isShot_ && motionState_ != DAMAGE && motionState_ != DEAD) {
		if (shotInterval_ > 0) {
			shotInterval_--;
			if (shotInterval_ <= 0) {
				shotInterval_ = 0;
			}

		} else {
			isShot_ = true;
			shotInterval_ = kMaxShotInterval;
		}
	}

	if (hitPoint_ <= 0) {
		if (motionState_ != DEAD) {
			motionState_ = DEAD;
			transform_.scale = { 5.0f,5.0f,5.0f };
		}

		if (transform_.scale.Length() < 0.01f) {
			isActive_ = false;
		}
	}

	model_.transform_ = transform_;
	model_.Update();
}

void Enemy::Draw(Camera* camera) {
	if (!isActive_) {
		return;
	}

	model_.Draw(camera);
}

void Enemy::OnCollision(const nlohmann::json& otherData) {
	if (motionState_ == DAMAGE || motionState_ == DEAD) {
		return;
	}

	if (otherData.contains("Attack")) {
		hitPoint_ -= otherData["Attack"].get<int>();
		motionTime_ = 0.3f;
		motionState_ = DAMAGE;
	}

	shotInterval_ = kMaxShotInterval;
}

void Enemy::HitRevenge(int level) {
	if (motionState_ == DEAD) {
		return;
	}

	if (level <= 0) {
		return;
	}

	if (isActive_) {
		motionState_ = DAMAGE;
		motionTime_ = 0.5f;

		int damage = maxHitPoint_ / (4 - level);
		hitPoint_ -= damage;

		DebugLog(std::format("hp: {}/{}", hitPoint_, maxHitPoint_));

		shotInterval_ = kMaxShotInterval;
	}
}

void Enemy::ChangeState(std::unique_ptr<BaseEnemyState> state) {
	state_ = std::move(state);
}

void Enemy::Approch() {
	switch (moveType_)
	{
	case MoveType::Normal:
		transform_.translate += velocity_ * engineCore_->GetDeltaTime();
		break;
	case MoveType::Sin:
		transform_.translate += velocity_ * engineCore_->GetDeltaTime();
		velocity_.y += sinf(frameCount_ * engineCore_->GetDeltaTime()) * 0.01f;
		break;
	default:
		break;
	}

	if (transform_.translate.z <= 0.0f) {
		phase_ = Phase::Leave;
	}
}

void Enemy::Leave() {
	if (transform_.translate.Normalize().x == 0.0f) {
		transform_.translate.x += leaveSpeed_ * engineCore_->GetDeltaTime();
	} else {
		transform_.translate.x += transform_.translate.Normalize().x * engineCore_->GetDeltaTime() * leaveSpeed_;
	}
}

void Enemy::Shot() {
	isShot_ = true;
	transform_.scale = { 1.5f,1.5f,1.5f };
}

void Enemy::Spawn(Vector3 position, Vector3 velocity, uint32_t moveType) {
	if (!isActive_) {
		transform_.scale = { 0.0f,0.0f,0.0f };
		isActive_ = true;
		transform_.translate = position;
		velocity_ = velocity;
	} else {
		assert(false && "Enemy is already active. Cannot spawn again.");
	}

	moveType_ = static_cast<MoveType>(moveType);
}

void Enemy::SetIsShot(bool isShot) {
	isShot_ = isShot;
}

void Enemy::SetIsActive(bool isActive) {
	isActive_ = isActive;
}

bool Enemy::GetIsActive() {
	return isActive_;
}

Phase Enemy::GetPhase() {
	return phase_;
}

bool Enemy::GetIsShot() {
	return isShot_;
}

Matrix4x4 Enemy::GetRotateMatrix() {
	return Matrix4x4::MakeRotateXYZMatrix(transform_.rotate);
}

Vector3 Enemy::GetDir() {
	return Vector3::Transform({ 0.0f,0.0f,1.0f }, Matrix4x4::MakeRotateXYZMatrix(model_.transform_.rotate));
}

Vector3 Enemy::GetWorldPosition() {
	return Vector3::Transform(Vector3::Zero(), Matrix4x4::MakeAffineMatrix(model_.transform_.scale, model_.transform_.rotate, model_.transform_.translate));
}

void Enemy::NormalMotion() {
	float shotRate = 1.0f - (static_cast<float>(shotInterval_) / static_cast<float>(kMaxShotInterval));
	transform_.rotate.z += shotRate * 0.5f;

	Eas::SimpleEaseIn(&transform_.scale.x, 1.0f + shotRate * 0.5f, 0.1f);
	Eas::SimpleEaseIn(&transform_.scale.y, 1.0f + shotRate * 0.5f, 0.1f);
	Eas::SimpleEaseIn(&transform_.scale.z, 1.0f + shotRate * 0.5f, 0.1f);

	Eas::SimpleEaseIn(&model_.material_.materialData_->color.x, shotRate, 0.1f);
	Eas::SimpleEaseIn(&model_.material_.materialData_->color.y, 1.0f, 0.1f);
	Eas::SimpleEaseIn(&model_.material_.materialData_->color.z, 1.0f, 0.1f);
	Eas::SimpleEaseIn(&model_.material_.materialData_->color.w, 1.0f, 0.1f);

	motionTime_ = 0.0f;
}

void Enemy::DamageMotion() {
	Eas::SimpleEaseIn(&model_.material_.materialData_->color.x, 1.0f, 0.1f);
	Eas::SimpleEaseIn(&model_.material_.materialData_->color.y, 0.0f, 0.1f);
	Eas::SimpleEaseIn(&model_.material_.materialData_->color.z, 0.0f, 0.1f);
	Eas::SimpleEaseIn(&model_.material_.materialData_->color.w, 1.0f, 0.1f);

	if (static_cast<int>(frameCount_) % 2 == 0) {
		model_.material_.materialData_->color.x = 1.0f;
		model_.material_.materialData_->color.y = 0.0f;
		model_.material_.materialData_->color.z = 0.0f;
		model_.material_.materialData_->enableLighting = false;
	} else {
		model_.material_.materialData_->color.x = 1.0f;
		model_.material_.materialData_->color.y = 1.0f;
		model_.material_.materialData_->color.z = 1.0f;
		model_.material_.materialData_->enableLighting = true;
	}
	transform_.scale.x = sinf(static_cast<float>(frameCount_)) * 0.3f + 0.7f;
	transform_.scale.y = cosf(static_cast<float>(frameCount_)) * 0.3f + 0.7f;
	transform_.scale.z = sinf(static_cast<float>(frameCount_)) * 0.3f + 0.7f;

	if (motionTime_ <= 0.0f) {
		motionState_ = NORMAL;
	} else {
		motionTime_ -= engineCore_->GetDeltaTime();
	}
}

void Enemy::DeadMotion() {
	model_.material_.materialData_->color.x = 1.0f;
	model_.material_.materialData_->color.y = 0.0f;
	model_.material_.materialData_->color.z = 0.0f;
	model_.material_.materialData_->enableLighting = false;

	Eas::SimpleEaseIn(&transform_.scale.x, 0.0f, 0.1f);
	Eas::SimpleEaseIn(&transform_.scale.y, 0.0f, 0.1f);
	Eas::SimpleEaseIn(&transform_.scale.z, 0.0f, 0.1f);

	transform_.rotate.x += engineCore_->GetDeltaTime() * 20.0f;
	transform_.rotate.y += engineCore_->GetDeltaTime() * 20.0f;
}
