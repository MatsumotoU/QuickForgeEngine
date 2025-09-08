#include "ParticleManager.h"

void ParticleManager::Initialize(EngineCore* engineCore, Camera* camera) {
	particle_.Initialize(engineCore, kMaxParticle);
	particle_.LoadModel("Resources/Model/particle", "MiniCube.obj", COORDINATESYSTEM_HAND_LEFT);
	camera_ = camera;
	engineCore_ = engineCore;
	particleData_.resize(kMaxParticle);
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
				float speed = static_cast<float>((rand() % power)+1);
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