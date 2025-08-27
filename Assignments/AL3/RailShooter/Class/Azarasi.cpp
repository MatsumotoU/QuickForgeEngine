#include "Azarasi.h"
#include "Utility/MyEasing.h"

void Azarasi::Initialize(EngineCore* engineCore) {
	engineCore_ = engineCore;
	normalModel_.Initialize(engineCore_);
	normalModel_.LoadModel("Resources", "Azarasi.obj", COORDINATESYSTEM_HAND_RIGHT);
	openMouthModel_.Initialize(engineCore_);
	openMouthModel_.LoadModel("Resources", "AzarasiOpen.obj", COORDINATESYSTEM_HAND_RIGHT);

	reqestMouthOpen_ = false;
	isMouthOpen_ = false;
}

void Azarasi::Update() {
	// 口空け遷移
	if (reqestMouthOpen_ != isMouthOpen_) {
		Eas::SimpleEaseIn(&transform_.scale.y, 0.0f, 0.5f);
		if (transform_.scale.y <= 0.1f) {
			transform_.scale.y = 0.0f;
			isMouthOpen_ = reqestMouthOpen_;
		}
	} else {
		Eas::SimpleEaseIn(&transform_.scale.x, 1.0f, 0.3f);
		Eas::SimpleEaseIn(&transform_.scale.y, 1.0f, 0.5f);
		Eas::SimpleEaseIn(&transform_.scale.z, 1.0f, 0.3f);
		if (transform_.scale.y >= 0.9f) {
			transform_.scale.y = 1.0f;
		}
	}

	// 移動処理
	normalModel_.transform_ = transform_;
	openMouthModel_.transform_ = transform_;

	normalModel_.Update();
	openMouthModel_.Update();
}

void Azarasi::Draw(Camera* camera) {
	if (isMouthOpen_) {
		openMouthModel_.Draw(camera);
	} else {
		normalModel_.Draw(camera);
	}
}
