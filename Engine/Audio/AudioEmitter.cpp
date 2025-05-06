#include "AudioEmitter.h"

AudioEmitter::AudioEmitter() {
	position_ = {}; // 音源の位置
	velocity_ = {}; // 音源の速度
	orientFront_ = {}; // 音源の横方向の面の向き
	orientTop_ = {}; // 音源の縦方向の面の向き
	nChannels_ = 2; // ソースボイスのチャンネル数
	channelRadius_ = 1.0f; // 放射半径 (通常は 1.0)
	curveDistanceScaler_ = 1.0f;// 距離減衰のスケーラー
	dopplerScaler_ = 1.0f;

	emitter_ = {0};
	channelAzimuths_.clear();
	channelAzimuths_.push_back(0.0f);
	channelAzimuths_.push_back(0.0f);
}

X3DAUDIO_EMITTER* AudioEmitter::GetEmitter() {

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

	emitter_ = { 0 };
	emitter_.Position = pos; // 音源の座標
	emitter_.Velocity = vel; // 音源の速度
	emitter_.OrientFront = orientFront; // 音源の前方ベクトル (指向性がある場合)
	emitter_.OrientTop = orientTop;   // 音源の上方ベクトル (指向性がある場合)
	emitter_.pCone = nullptr;                  // 指向性コーン (ある場合)
	emitter_.ChannelCount = nChannels_; // 音源のチャンネル数
	emitter_.ChannelRadius = channelRadius_;              // 放射半径 (通常は 1.0)

	channelAzimuths_.clear();
	for (uint32_t i = 0; i < nChannels_; i++) {
		channelAzimuths_.push_back(0.0f);
	}
	emitter_.pChannelAzimuths = channelAzimuths_.data();        // 各チャンネルの方向 (マルチチャンネル音源の場合)
	emitter_.CurveDistanceScaler = curveDistanceScaler_;        // 距離減衰のスケーラー
	emitter_.DopplerScaler = dopplerScaler_;
	
	return &emitter_;
}
