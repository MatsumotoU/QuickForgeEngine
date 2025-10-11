#pragma once
#include "MathInclude.h"
#include <random>

namespace MyMath {
	float Leap(float a, float b, float t);
	void SimpleEaseIn(float* value, float endValue, float transitionSpeed);
	float DegreesToRadians(float degrees);

	template<typename T>
	inline T EaseIn(T from, T to, float t) {
		return from + (to - from) * t * t;
	}
	template<typename T>
	inline T EaseOut(T from, T to, float t) {
		return from + (to - from) * (1 - (1 - t) * (1 - t));
	}
	template<typename T>
	inline T EaseInOut(T from, T to, float t) {
		if (t < 0.5f) {
			return EaseIn(from, (from + to) / 2, t * 2);
		} else {
			return EaseOut((from + to) / 2, to, (t - 0.5f) * 2);
		}
	}
	inline float Rand(float min, float max) {
		static std::random_device rd;
		static std::mt19937 mt(rd());
		std::uniform_real_distribution<float> dist(min, max);
		return dist(mt);
	}
}