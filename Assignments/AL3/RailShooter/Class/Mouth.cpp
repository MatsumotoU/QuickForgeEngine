#include "Mouth.h"

void Mouth::Initialize(EngineCore* engineCore) {
	engineCore_ = engineCore;
	openRadian_ = -1.0f;
	topMouthModel_.Initialize(engineCore_);
	topMouthModel_.LoadModel("Resources", "mouth.obj", COORDINATESYSTEM_HAND_RIGHT);
	underMouthModel_.Initialize(engineCore_);
	underMouthModel_.LoadModel("Resources", "mouth.obj", COORDINATESYSTEM_HAND_RIGHT);
	topMouthTransform_.translate = { 0.0f,0.1f,0.0f };
	topMouthTransform_.rotate = { openRadian_,0.0f,3.14f };
	topMouthTransform_.scale = { 1.0f,1.0f,1.0f };
	underMouthTransform_.translate = { 0.0f,-0.1f,0.0f };
	underMouthTransform_.rotate = { openRadian_,0.0f,0.0f };
	underMouthTransform_.scale = { 1.0f,1.0f,1.0f };
}

void Mouth::Update() {
	Transform top = transform_;
	Transform under = transform_;
	top.translate += topMouthTransform_.translate;
	top.rotate += topMouthTransform_.rotate;
	top.scale *= topMouthTransform_.scale;
	top.rotate.x = openRadian_;
	under.translate += underMouthTransform_.translate;
	under.rotate += underMouthTransform_.rotate;
	under.scale *= underMouthTransform_.scale;
	under.rotate.x = openRadian_;

	topMouthModel_.transform_ = top;
	underMouthModel_.transform_ = under;
	topMouthModel_.Update();
	underMouthModel_.Update();
}

void Mouth::Draw(Camera* camera) {
	if (transform_.scale.x <= 0.1f) {
		return;
	};
	topMouthModel_.Draw(camera);
	underMouthModel_.Draw(camera);
}
