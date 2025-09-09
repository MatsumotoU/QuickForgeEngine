#pragma once
#include "Base/EngineCore.h"
#include<memory>
static inline const float kFadeInTime = 20.0f;
static inline const int kSpalateNum = 10;

class Fade {
public:
	Fade() = delete;
	Fade(EngineCore* engineCore);

	void Initialize();
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
	std::vector<std::unique_ptr<Sprite>> sprites_;
	Vector4 color_ = {1.0f, 1.0f, 1.0f, 1.0f};

	float fadeTime_;
	EngineCore* engineCore_;
	Camera camera_;
	Transform transform_;
};