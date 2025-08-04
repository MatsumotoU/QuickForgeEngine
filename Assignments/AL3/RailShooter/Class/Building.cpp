#include "Building.h"
#include <random>
#include "Utility/MyEasing.h"

void Building::Init(EngineCore* engineCore) {
    engineCore_ = engineCore;
	buillding_.Initialize(engineCore,64);
	buillding_.LoadModel("Resources", "Box.obj", COORDINATESYSTEM_HAND_RIGHT);
    buillding_.material_.materialData_->color = Vector4(0.3f, 0.3f, 0.3f, 1.0f);
    for (int i = 0; i < 64; i++) {
        transform_.push_back(Transform());
    }

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> distX(-3.0f, 3.0f);
    std::uniform_real_distribution<float> distS(0.5f, 2.0f);

    const float yValue = -3.0f;
    const size_t count = transform_.size();
    const float zStart = 0.0f;
    const float zEnd = 100.0f;
    const float zStep = (count > 1) ? (zEnd - zStart) / (count - 1) : 0.0f;

    for (size_t i = 0; i < count; ++i) {
        transform_[i].translate.x = distX(gen);
        transform_[i].translate.y = yValue;
        transform_[i].translate.z = zStart + zStep * i;

        transform_[i].scale.x = distS(gen);
        transform_[i].scale.y = distS(gen);
        transform_[i].scale.z = distS(gen);
    }
}

void Building::Update() {
    for (Transform& t : transform_) {
        t.translate.z -= engineCore_->GetDeltaTime() * 2.0f;
        if (t.translate.z <= 0.0f) {
            t.translate.z = 100.0f;
        }

        if (static_cast<int>(t.translate.z) % 10) {
            t.scale = Vector3(2.0f, 2.0f, 2.0f);
        }

        Eas::SimpleEaseIn(&t.scale.x, 1.0f, 0.1f);
        Eas::SimpleEaseIn(&t.scale.y, 1.0f, 0.1f);
        Eas::SimpleEaseIn(&t.scale.z, 1.0f, 0.1f);
    }
}

void Building::Draw(Camera* camera) {
	buillding_.Draw(&transform_, camera);
}
