#include "Bulllet.h"

void Bullet::Initialize(EngineCore* engineCore, const std::string& name) {
	name_ = name;
	engineCore_ = engineCore;
	model_.Initialize(engineCore_);
	if (name_ == "Player") {
		model_.LoadModel("Resources", "Bullet.obj", COORDINATESYSTEM_HAND_RIGHT);
	} else {
		ice_.Initialize(engineCore_);
	}
	
	isActive_ = false;

	transform_.scale = Vector3(0.5f, 0.5f, 0.5f);

	objData_["Attack"] = 10;
	isHoming_ = false;
}

void Bullet::Update() {
	if (isActive_) {
		transform_.translate += velocity_ * engineCore_->GetDeltaTime();
		transform_.rotate = -Vector3::LookAt(transform_.translate, transform_.translate + velocity_);

		if (aliveTime_ > 0) {
			aliveTime_--;
		} else {
			isActive_ = false;
		}
	}
	
	if (name_ == "Enemy") {
		ice_.transform_.translate = transform_.translate;
		ice_.transform_.scale = transform_.scale;
		
		if (isHoming_) {
			ice_.transform_.rotate.x += 0.4f;
		} else {
			ice_.transform_.rotate.x += 0.2f;
			ice_.transform_.rotate.y += 0.1f;
		}

		ice_.Update();
	} else {
		model_.transform_ = transform_;
		model_.Update();
	}

	objData_["Position"] = {
		{"x",GetWorldPosition().x},
		{"y",GetWorldPosition().y},
		{"z",GetWorldPosition().z}
	};
}

void Bullet::Draw(Camera* camera) {
	if (isActive_) {
		if (name_ == "Enemy") {
			ice_.Draw(camera);
		} else {
			model_.Draw(camera);
		}
	}
}

void Bullet::ShotBullet(Vector3 position, Vector3 velocity, uint32_t aliveTime, uint32_t attack) {
	transform_.translate = position;
	velocity_ = velocity;
	aliveTime_ = aliveTime;
	maxAliveTime_ = aliveTime;
	isActive_ = true;
	isHoming_ = false;
	objData_["Attack"] = attack;
	SetRadius(static_cast<float>(attack) / 20.0f);

	transform_.scale.x = static_cast<float>(objData_["Attack"]) / 20.0f;
	transform_.scale.y = static_cast<float>(objData_["Attack"]) / 20.0f;
	transform_.scale.z = static_cast<float>(objData_["Attack"]) / 20.0f;

	if (name_ == "Enemy") {
		ice_.syropeColor_ = { position.x,position.y,position.z,1.0f };
	}
}

void Bullet::OnCollision(const nlohmann::json& otherData) {
	otherData;
	isActive_ = false;
}

bool Bullet::GetIsActive() {
	return isActive_;
}

Vector3 Bullet::GetWorldPosition() {
	return transform_.translate;
}

void Bullet::SetName(const std::string& name) {
	name_ = name;
}
