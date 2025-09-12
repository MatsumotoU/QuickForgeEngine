#include "MyMath.h"

float MyMath::Leap(float a, float b, float t) {
    return a * t + b * (1.0f - t);
}

void MyMath::SimpleEaseIn(float* value, float endValue, float transitionSpeed) {
	*value += (endValue - *value) * transitionSpeed;

	if (fabsf(*value - endValue) <= 0.01f) {
		*value = endValue;
	}
}
