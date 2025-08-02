#include "MyEasing.h"
#include <math.h>

float Eas::Liner(float transitionRate, float startValue, float endValue) {

	return ((1.0f - transitionRate) * startValue) + (endValue * transitionRate);
}

float Eas::EaseIn(float transitionRate, float exponent, float startValue, float endValue) {
	return (1.0f - powf(transitionRate, exponent)) * startValue + powf(transitionRate, exponent) * endValue;
}

float Eas::EaseOut(float transitionRate, float exponent, float startValue, float endValue) {
	return  (1.0f - (1.0f - powf(1.0f - transitionRate, exponent))) * startValue + (1.0f - powf(1.0f - transitionRate, exponent)) * endValue;
}

float Eas::EaseInOutQuart(float transitionRate, float startValue, float endValue) {
	if (transitionRate < 0.5f) {
		return  (1.0f - powf(transitionRate, 4.0f) * 8.0f) * startValue + powf(transitionRate, 4.0f) * 8.0f * endValue;
	} else {
		return  (1.0f - (1.0f - powf(-2.0f * transitionRate + 2.0f, 4.0f) / 2.0f)) * startValue + (1.0f - powf(-2.0f * transitionRate + 2.0f, 4.0f) / 2.0f) * endValue;
	}
}

void Eas::SimpleEaseIn(float* value, float endValue, float transitionSpeed) {

	*value += (endValue - *value) * transitionSpeed;

	if (fabsf(*value - endValue) <= 0.01f) {
		*value = endValue;
	}

}

void Eas::SimpleFadeOut(unsigned int* color) {

	if (*color > 0xFFFFFF00) {
		color -= 0x0000000A;
	}
}
