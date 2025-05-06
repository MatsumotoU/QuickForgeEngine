#include "AudioListener.h"

AudioListener::AudioListener() {
	position_ = {};
	velocity_ = {};
	orientFront_ = {0.0f,0.0f,-1.0f};
	orientTop_ = {0.0f,1.0f,0.0f};

	listener_ = {};
}

X3DAUDIO_LISTENER* AudioListener::GetListener() {
	X3DAUDIO_VECTOR pos{};
	pos.x = position_.x;
	pos.y = position_.y;
	pos.z = position_.z;

	X3DAUDIO_VECTOR vel{};
	vel.x = velocity_.x;
	vel.y = velocity_.y;
	vel.z = velocity_.z;

	X3DAUDIO_VECTOR orientFront{};
	orientFront.x = orientFront_.x;
	orientFront.y = orientFront_.y;
	orientFront.z = orientFront_.z;

	X3DAUDIO_VECTOR orientTop{};
	orientTop.x = orientTop_.x;
	orientTop.y = orientTop_.y;
	orientTop.z = orientTop_.z;

	listener_ = {};
	listener_.Position = pos; // 音源の座標
	listener_.Velocity = vel; // 音源の速度
	listener_.OrientFront = orientFront; // 音源の前方ベクトル (指向性がある場合)
	listener_.OrientTop = orientTop;   // 音源の上方ベクトル (指向性がある場合)

	return &listener_;
}