#pragma once
#include "../../../../Engine/Base/EngineCore.h"

static inline const float kFadeInTime = 20.0f;

class Fade {
public:
	void Initialize(EngineCore* engineCore);
	void Update();
	void Draw();

public:
	void StartFadeIn();
	void StartFadeOut();

	bool isFadeIn_;
	bool isFadeOut_;

	bool isEndFadeIn_;
	bool isEndFadeOut_;

public:
	bool isFading();

private:
	uint32_t uvCheckerTextureHandle_;
	Vector4 color_ = { 1.0f, 1.0f, 1.0f, 1.0f };
	Sprite sprite_;

	float fadeTime_;
	EngineCore* engineCore_;
	Camera camera_;
	Transform transform_;
};