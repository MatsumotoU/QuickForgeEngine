#pragma once
#include "Base/EngineCore.h"

class ControlUI final {
public:
	void Initialize(EngineCore* engineCore,Camera* camera);
	void Update();
	void Draw();

	Transform transform_;
	bool isActiveControll_;

private:
	float timer_ = 0.0f;
	float scalingTimer_ = 0.0f;
	EngineCore* engineCore_;
	Camera* camera_;
	std::unique_ptr<Model> controlTextModel_;

	std::unique_ptr<Model> controllerModel_;
	std::unique_ptr<Model> mouseModel_;

	Vector2 clickPos_;
	bool isClicked_ = false;
};