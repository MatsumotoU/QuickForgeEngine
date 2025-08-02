#pragma once

namespace Eas {

	float Liner(float transitionRate, float startValue, float endValue);

	float EaseIn(float transitionRate, float exponent, float startValue, float endValue);

    float EaseOut(float transitionRate, float exponent, float startValue, float endValue);

	float EaseInOutQuart(float transitionRate, float startValue, float endValue);

	void SimpleEaseIn(float* value, float endValue, float transitionSpeed);

	void SimpleFadeOut(unsigned int* color);
}



