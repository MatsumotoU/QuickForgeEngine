#pragma once
#include "Math/Vector/Vector4.h"
#include "Math/Vector/Vector3.h"

struct alignas(16) DirectionalLight
{
	Vector4 color;// ライトの色
	Vector3 direction;// ライトの向き
	float intensity;// 輝度
};