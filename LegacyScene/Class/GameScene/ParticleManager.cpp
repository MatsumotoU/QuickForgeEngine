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
		data.transform = Transform();
		data.velocity = Vector3(0.0f, 0.0f, 0.0f);
		data.color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
		data.lifeTime = 0.0f;
	}
}
