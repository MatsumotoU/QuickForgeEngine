#pragma once
#include <stdint.h>
#include <xaudio2.h>
#include <x3daudio.h>
#include <vector>

#include "../Math/Vector/Vector3.h"

class AudioEmitter final {
public:
	AudioEmitter();

public:
	X3DAUDIO_EMITTER* GetEmitter();

public:
	Vector3 position_; // 音源の位置
	Vector3 velocity_; // 音源の速度
	Vector3 orientFront_; // 音源の横方向の面の向き
	Vector3 orientTop_; // 音源の縦方向の面の向き
	uint32_t nChannels_; // ソースボイスのチャンネル数
	float channelRadius_; // 放射半径 (通常は 1.0)
	float curveDistanceScaler_;// 距離減衰のスケーラー
	float dopplerScaler_;

private:
	std::vector<float> channelAzimuths_;
	X3DAUDIO_EMITTER emitter_;

};