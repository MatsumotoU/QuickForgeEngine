#pragma once
#include <xaudio2.h>
#include <x3daudio.h>

#include "../Math/Vector/Vector3.h"

class AudioListener final{
public:
	AudioListener();

public:
	X3DAUDIO_LISTENER* GetListener();

public:
	Vector3 position_; // リスナーの位置
	Vector3 velocity_; // リスナーの速度
	Vector3 orientFront_; // リスナーの横方向の面の向き
	Vector3 orientTop_; // リスナーの縦方向の面の向き

private:
	X3DAUDIO_LISTENER listener_;
};