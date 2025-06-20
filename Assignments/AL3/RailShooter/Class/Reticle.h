#pragma once
#include "Base/EngineCore.h"
#include "Player.h"

class Reticle {
public:
	void Initialize(EngineCore* engineCore);
	void Update();
	void Draw(Camera* camera);

public:
	void SetPlayer(Player* player);
	Vector3 GetWorldPos();

public:
	Transform transform_;
	Model model_;

private:
	EngineCore* engineCore_;
	bool isActive_;
	Sprite sprite_;
	uint32_t reticleGH_;
	Transform spriteTransform_;

private:
	Player* player_;
};