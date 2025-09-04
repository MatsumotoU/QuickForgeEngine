#pragma once
#include "GamePlayer.h"

class Enemy final :public GamePlayer{
public:
	Enemy() = default;
	~Enemy() = default;
	void Initialize(EngineCore* engineCore, Camera* camera) override;
	void Update() override;
	void Draw() override;
};