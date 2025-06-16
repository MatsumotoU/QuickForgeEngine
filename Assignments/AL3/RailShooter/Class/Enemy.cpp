#include "Enemy.h"
#include "EnemyStateAproach.h"
#include "EnemyStateLeave.h"

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

	ChangeState(std::make_unique<EnemyStateAproach>(this));
}

void Enemy::Update() {
	if (!isActive_) {
		return;
	}

	frameCount_++;

	state_.get()->Update();

	if (fabsf(transform_.translate.x) >= kXLimit) {
		isActive_ = false;
	}

	timedCalls_.remove_if([](TimeCall* call) {
		if (call->IsFinished()) {
			delete call;
			return true;
		}
		return false;
		});

	for (TimeCall* timedCall:timedCalls_) {
		timedCall->Update();
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
	timedCalls_.push_back(new TimeCall(engineCore_, std::bind(&Enemy::Shot, this), 1.5f));
}

void Enemy::Spawn(Vector3 position, Vector3 velocity, uint32_t moveType) {
	if (!isActive_) {
		isActive_ = true;
		transform_.translate = position;
		velocity_ = velocity;
		Shot();
	} else {
		assert(false && "Enemy is already active. Cannot spawn again.");
	}

	moveType_ = static_cast<MoveType>(moveType);
}

void Enemy::SetIsShot(bool isShot) {
	isShot_ = isShot;
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
