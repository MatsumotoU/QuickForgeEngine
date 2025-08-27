#include "ExprotionParticle.h"

void ExprotionParticle::Init(EngineCore* engineCore) {
    engineCore_ = engineCore;
    particle.Initialize(engineCore, 64);
    particle.LoadModel("Resources", "Box.obj", COORDINATESYSTEM_HAND_RIGHT);
    particle.material_.materialData_->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
    for (int i = 0; i < 64; i++) {
        transform_.push_back(Transform());
    }
	isActive_ = false;
	life_ = 0;
}

void ExprotionParticle::Update() {
	if (!isActive_) return;
	if (life_ <= 0) {
		isActive_ = false;
		return;
	} else {
		life_--;
	}

	for (Transform& t : transform_) {
        t.translate += Vector3::Transform({ 0.0f,0.0f,1.0f }, Matrix4x4::MakeRotateXYZMatrix(t.rotate));
	}
}

void ExprotionParticle::Draw(Camera* camera) {
	if (!isActive_) return;
    particle.Draw(&transform_, camera);
}

void ExprotionParticle::Generate(const Vector3& position) {
	for (Transform& t : transform_) {
		t = Transform();
		t.translate = position;
		t.rotate.x = static_cast<float>(rand() % 360) * (3.14f / 180.0f);
		t.rotate.y = static_cast<float>(rand() % 360) * (3.14f / 180.0f);
		t.rotate.z = static_cast<float>(rand() % 360) * (3.14f / 180.0f);
		float speed = static_cast<float>((rand() % 100) / 100.0f * 0.1f + 0.05f);
		t.scale = Vector3(speed, speed, speed);
		isActive_ = true;
		life_ = 60;
	}
}
