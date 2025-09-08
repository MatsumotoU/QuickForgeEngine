#pragma once
#include "GamePlayer.h"

class Player final :public GamePlayer {
public:
	Player() = default;
	~Player() = default;
	void Initialize(EngineCore* engineCore, Camera* camera) override;
	void Update() override;
	void Draw() override;

private:
	void MouseControl();
	void ControllerControl();

	bool isClicked_;
	Vector2 clickStartPos_;

	float timer_;
};