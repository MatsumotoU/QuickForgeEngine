#include "ParticleManager.h"

void ParticleManager::Initialize(EngineCore* engineCore, Camera* camera) {
	particle_.Initialize(engineCore, kMaxParticle);
	particle_.LoadModel("Resources/Model/particle", "MiniCube.obj", COORDINATESYSTEM_HAND_LEFT);
	bako_.Initialize(engineCore, kMaxOnomatope);
	bako_.LoadModel("Resources/Model/UI", "BakoText.obj", COORDINATESYSTEM_HAND_RIGHT);
	doka_.Initialize(engineCore, kMaxOnomatope);
	doka_.LoadModel("Resources/Model/UI", "DokaText.obj", COORDINATESYSTEM_HAND_RIGHT);
	zudon_.Initialize(engineCore, kMaxOnomatope);
	zudon_.LoadModel("Resources/Model/UI", "ZudonText.obj", COORDINATESYSTEM_HAND_RIGHT);
	byon_.Initialize(engineCore, kMaxOnomatope);
	byon_.LoadModel("Resources/Model/UI", "ByonnText.obj", COORDINATESYSTEM_HAND_RIGHT);
	camera_ = camera;
	engineCore_ = engineCore;
	particleData_.resize(kMaxParticle);
	bakoData_.resize(kMaxOnomatope);
	dokaData_.resize(kMaxOnomatope);
	zudonData_.resize(kMaxOnomatope);
	byonData_.resize(kMaxOnomatope);
	ResetParticle();
}

void ParticleManager::Update() {
	for (auto& data : particleData_) {
		if (data.isActive) {
			if (data.lifeTime > 0.0f) {
				data.lifeTime -= engineCore_->GetDeltaTime();

				if (data.isGravity) {
					data.velocity.y -= 9.8f * engineCore_->GetDeltaTime();
				}

				data.transform.translate += data.velocity * engineCore_->GetDeltaTime();
				data.velocity = data.velocity * data.velocityDecay;

			} else {
				data.isActive = false;
			}
		}
	}

	for (auto& data : bakoData_) {
		if (data.isActive) {
			if (data.lifeTime > 0.0f) {
				data.lifeTime -= engineCore_->GetDeltaTime();
				if (data.isGravity) {
					data.velocity.y -= 9.8f * engineCore_->GetDeltaTime();
				}
				data.transform.translate += data.velocity * engineCore_->GetDeltaTime();
				data.velocity = data.velocity * data.velocityDecay;
			} else {
				data.isActive = false;
			}
		}
	}

	for (auto& data : dokaData_) {
		if (data.isActive) {
			if (data.lifeTime > 0.0f) {
				data.lifeTime -= engineCore_->GetDeltaTime();
				if (data.isGravity) {
					data.velocity.y -= 9.8f * engineCore_->GetDeltaTime();
				}
				data.transform.translate += data.velocity * engineCore_->GetDeltaTime();
				data.velocity = data.velocity * data.velocityDecay;
			} else {
				data.isActive = false;
			}
		}
	}

	for (auto& data : zudonData_) {
		if (data.isActive) {
			if (data.lifeTime > 0.0f) {
				data.lifeTime -= engineCore_->GetDeltaTime();
				if (data.isGravity) {
					data.velocity.y -= 9.8f * engineCore_->GetDeltaTime();
				}
				data.transform.translate += data.velocity * engineCore_->GetDeltaTime();
				data.velocity = data.velocity * data.velocityDecay;
			} else {
				data.isActive = false;
			}
		}
	}

	for (auto& data : byonData_) {
		if (data.isActive) {
			if (data.lifeTime > 0.0f) {
				data.lifeTime -= engineCore_->GetDeltaTime();
				if (data.isGravity) {
					data.velocity.y -= 9.8f * engineCore_->GetDeltaTime();
				}
				data.transform.translate += data.velocity * engineCore_->GetDeltaTime();
				data.velocity = data.velocity * data.velocityDecay;
			} else {
				data.isActive = false;
			}
		}
	}

	std::vector<Transform> transforms;
	transforms.clear();
	for (const auto& data : particleData_) {
		if (data.isActive) {
			transforms.push_back(data.transform);
		} else {
			Transform temp;
			temp.translate.y = 1000.0f; // 非アクティブなパーティクルを画面外に移動
			transforms.push_back(temp);
		}
	}
	particle_.Update(&transforms);

	std::vector<Transform> bakoTransforms;
	bakoTransforms.clear();
	for (const auto& data : bakoData_) {
		if (data.isActive) {
			bakoTransforms.push_back(data.transform);
		} else {
			Transform temp;
			temp.translate.y = 1000.0f; // 非アクティブなパーティクルを画面外に移動
			bakoTransforms.push_back(temp);
		}
	}
	bako_.Update(&bakoTransforms);

	std::vector<Transform> dokaTransforms;
	dokaTransforms.clear();
	for (const auto& data : dokaData_) {
		if (data.isActive) {
			dokaTransforms.push_back(data.transform);
		} else {
			Transform temp;
			temp.translate.y = 1000.0f; // 非アクティブなパーティクルを画面外に移動
			dokaTransforms.push_back(temp);
		}
	}
	doka_.Update(&dokaTransforms);

	std::vector<Transform> zudonTransforms;
	zudonTransforms.clear();
	for (const auto& data : zudonData_) {
		if (data.isActive) {
			zudonTransforms.push_back(data.transform);
		} else {
			Transform temp;
			temp.translate.y = 1000.0f; // 非アクティブなパーティクルを画面外に移動
			zudonTransforms.push_back(temp);
		}
	}
	zudon_.Update(&zudonTransforms);

	std::vector<Transform> byonTransforms;
	byonTransforms.clear();
	for (const auto& data : byonData_) {
		if (data.isActive) {
			byonTransforms.push_back(data.transform);
		} else {
			Transform temp;
			temp.translate.y = 1000.0f; // 非アクティブなパーティクルを画面外に移動
			byonTransforms.push_back(temp);
		}
	}
	byon_.Update(&byonTransforms);
}

void ParticleManager::Draw() {
	std::vector<Vector4> colors;
	colors.clear();
	for (const auto& data : particleData_) {
		if (data.isActive) {
			colors.push_back(data.color);
		}
	}
	particle_.Draw(&colors, camera_);

	std::vector<Vector4> bakoColors;
	bakoColors.clear();
	for (const auto& data : bakoData_) {
		if (data.isActive) {
			bakoColors.push_back(data.color);
		}
	}
	bako_.Draw(&bakoColors, camera_);
	std::vector<Vector4> dokaColors;
	dokaColors.clear();
	for (const auto& data : dokaData_) {
		if (data.isActive) {
			dokaColors.push_back(data.color);
		}
	}
	doka_.Draw(&dokaColors, camera_);
	std::vector<Vector4> zudonColors;
	zudonColors.clear();
	for (const auto& data : zudonData_) {
		if (data.isActive) {
			zudonColors.push_back(data.color);
		}
	}
	zudon_.Draw(&zudonColors, camera_);
	std::vector<Vector4> byonColors;
	byonColors.clear();
	for (const auto& data : byonData_) {
		if (data.isActive) {
			byonColors.push_back(data.color);
		}
	}
	byon_.Draw(&byonColors, camera_);

}

void ParticleManager::ResetParticle() {
	for (auto& data : particleData_) {
		data.isActive = false;
		data.isGravity = false;
		data.transform = Transform();
		data.velocity = Vector3(0.0f, 0.0f, 0.0f);
		data.color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
		data.lifeTime = 0.0f;
		data.velocityDecay = 0.98f;
	}

	for (auto& data : bakoData_) {
		data.isActive = false;
		data.isGravity = false;
		data.transform = Transform();
		data.transform.rotate.y = 3.14f;
		data.velocity = Vector3(0.0f, 0.0f, 0.0f);
		data.color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
		data.lifeTime = 0.0f;
		data.velocityDecay = 0.98f;
	}

	for (auto& data : dokaData_) {
		data.isActive = false;
		data.isGravity = false;
		data.transform = Transform();
		data.transform.rotate.y = 3.14f;
		data.velocity = Vector3(0.0f, 0.0f, 0.0f);
		data.color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
		data.lifeTime = 0.0f;
		data.velocityDecay = 0.98f;
	}

	for (auto& data : zudonData_) {
		data.isActive = false;
		data.isGravity = false;
		data.transform = Transform();
		data.transform.rotate.y = 3.14f;
		data.velocity = Vector3(0.0f, 0.0f, 0.0f);
		data.color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
		data.lifeTime = 0.0f;
		data.velocityDecay = 0.98f;
	}

	for (auto& data : byonData_) {
		data.isActive = false;
		data.isGravity = false;
		data.transform = Transform();
		data.transform.rotate.y = 3.14f;
		data.velocity = Vector3(0.0f, 0.0f, 0.0f);
		data.color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
		data.lifeTime = 0.0f;
		data.velocityDecay = 0.98f;
	}
}

void ParticleManager::EmitBomb(const Vector3& position, const Vector4& color, int power, float lifeTime, float velocityDecay, int maxParticle) {
	for (int i = 0; i < maxParticle; i++) {
		for (auto& data : particleData_) {
			if (!data.isActive) {
				data.isActive = true;
				data.isGravity = true;
				data.transform.translate = position;
				data.transform.scale = Vector3(0.2f, 0.2f, 0.2f);
				float angle1 = static_cast<float>(rand() % 360) * (3.14f / 180.0f);
				float angle2 = static_cast<float>(rand() % 360) * (3.14f / 180.0f);
				float speed = static_cast<float>((rand() % power) + 1);
				data.velocity = Vector3(
					speed * cosf(angle1) * sinf(angle2),
					speed * cosf(angle2),
					speed * sinf(angle1) * sinf(angle2)
				);
				data.color = color;
				data.lifeTime = lifeTime;
				data.velocityDecay = velocityDecay;
				break;
			}
		}
	}
}

void ParticleManager::EmitBako(const Vector3& position, const Vector4& color, int power, float lifeTime) {
	for (auto& data : bakoData_) {
		if (!data.isActive) {
			data.isActive = true;
			data.isGravity = true;
			data.transform.translate = position;
			data.transform.scale = Vector3(1.0f, 1.0f, 1.0f);
			float angle1 = static_cast<float>(rand() % 360) * (3.14f / 180.0f);
			float angle2 = static_cast<float>(rand() % 360) * (3.14f / 180.0f);
			float speed = static_cast<float>((rand() % power) + 1);
			data.velocity = Vector3(
				speed * cosf(angle1) * sinf(angle2),
				static_cast<float>(power),
				speed * sinf(angle1) * sinf(angle2)
			);
			data.color = color;
			data.lifeTime = lifeTime;
			data.velocityDecay = 0.95f;
			break;
		}
	}
}

void ParticleManager::EmitDoka(const Vector3& position, const Vector4& color, int power, float lifeTime) {
	for (auto& data : dokaData_) {
		if (!data.isActive) {
			data.isActive = true;
			data.isGravity = true;
			data.transform.translate = position;
			data.transform.scale = Vector3(1.0f, 1.0f, 1.0f);
			float angle1 = static_cast<float>(rand() % 360) * (3.14f / 180.0f);
			float angle2 = static_cast<float>(rand() % 360) * (3.14f / 180.0f);
			float speed = static_cast<float>((rand() % power) + 1);
			data.velocity = Vector3(
				speed * cosf(angle1) * sinf(angle2),
				static_cast<float>(power),
				speed * sinf(angle1) * sinf(angle2)
			);
			data.color = color;
			data.lifeTime = lifeTime;
			data.velocityDecay = 0.95f;
			break;
		}
	}
}

void ParticleManager::EmitZudon(const Vector3& position, const Vector4& color, int power, float lifeTime) {
	for (auto& data : zudonData_) {
		if (!data.isActive) {
			data.isActive = true;
			data.isGravity = true;
			data.transform.translate = position;
			data.transform.scale = Vector3(1.0f, 1.0f, 1.0f);
			float angle1 = static_cast<float>(rand() % 360) * (3.14f / 180.0f);
			float angle2 = static_cast<float>(rand() % 360) * (3.14f / 180.0f);
			float speed = static_cast<float>((rand() % power) + 1);
			data.velocity = Vector3(
				speed * cosf(angle1) * sinf(angle2),
				static_cast<float>(power),
				speed * sinf(angle1) * sinf(angle2)
			);
			data.color = color;
			data.lifeTime = lifeTime;
			data.velocityDecay = 0.95f;
			break;
		}
	}

}

void ParticleManager::EmitByon(const Vector3& position, const Vector4& color, int power, float lifeTime) {
	for (auto& data : byonData_) {
		if (!data.isActive) {
			data.isActive = true;
			data.isGravity = true;
			data.transform.translate = position;
			data.transform.scale = Vector3(1.0f, 1.0f, 1.0f);
			float angle1 = static_cast<float>(rand() % 360) * (3.14f / 180.0f);
			float angle2 = static_cast<float>(rand() % 360) * (3.14f / 180.0f);
			float speed = static_cast<float>((rand() % power) + 1);
			data.velocity = Vector3(
				speed * cosf(angle1) * sinf(angle2),
				static_cast<float>(power),
				speed * sinf(angle1) * sinf(angle2)
			);
			data.color = color;
			data.lifeTime = lifeTime;
			data.velocityDecay = 0.95f;
			break;
		}
	}
}
