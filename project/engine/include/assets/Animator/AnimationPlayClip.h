#pragma once
#include "AnimClip.h"

namespace QFE {
	/// @brief アニメーションクリップの再生に必要な情報をまとめた構造体
    struct AnimationPlayClip {
        AnimClip* animClip;
        float playSpeed;
        float startTime;

		float currentTime;

		bool isLoop;
		bool isPlaying;
    };
}