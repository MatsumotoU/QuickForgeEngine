#include "ListenerObj.h"

void ListenerObj::Initialize(EngineCore* engineCore) {
	engineCore_ = engineCore;
	model_.Initialize(engineCore_);
	model_.LoadModel("Resources", "monkey.obj", COORDINATESYSTEM_HAND_RIGHT);

	transform_.rotate.y = 3.14f;
}

void ListenerObj::Update() {
}

void ListenerObj::Draw(Camera* camera) {
	model_.Draw(transform_, camera);
}
