#include "MyEasing.h"
#include <cmath>

float MyEasing::Liner(float transitionRate, float startValue, float endValue) {

	return ((1.0f - transitionRate) * startValue) + (endValue * transitionRate);
}

float MyEasing::EaseIn(float transitionRate, float exponent, float startValue, float endValue) {
	return (1.0f - std::powf(transitionRate, exponent)) * startValue + std::powf(transitionRate, exponent) * endValue;
}

float MyEasing::EaseOut(float transitionRate, float exponent, float startValue, float endValue) {
	return  (1.0f - (1.0f - std::powf(1.0f - transitionRate, exponent))) * startValue + (1.0f - std::powf(1.0f - transitionRate, exponent)) * endValue;
}

float MyEasing::EaseInOutQuart(float transitionRate, float startValue, float endValue) {
	if (transitionRate < 0.5f) {
		return  (1.0f - std::powf(transitionRate, 4.0f) * 8.0f) * startValue + std::powf(transitionRate, 4.0f) * 8.0f * endValue;
	} else {
		return  (1.0f - (1.0f - std::powf(-2.0f * transitionRate + 2.0f, 4.0f) / 2.0f)) * startValue + (1.0f - std::powf(-2.0f * transitionRate + 2.0f, 4.0f) / 2.0f) * endValue;
	}
}

void MyEasing::SimpleEaseIn(float* value, float endValue, float transitionSpeed) {

	*value += (endValue - *value) * transitionSpeed;

	if (std::fabsf(*value - endValue) <= 0.01f) {
		*value = endValue;
	}

}

void MyEasing::SimpleFadeOut(unsigned int* color) {

	if (*color > 0xFFFFFF00) {
		color -= 0x0000000A;
	}
}
