#pragma once
#include "Core/Math/Vector/Vector2.h"

struct alignas(16) VignetteOffset {
	Vector2 screenResolution; // スクリーンの解像度
	float VignetteRadius;  // ビネットの中心からの距離
	float VignetteSoftness;   // ビネットの境界の柔らかさ
	float VignetteIntensity; // ビネットの強さ
	float padding[3]; // アライメントのためのパディング
};