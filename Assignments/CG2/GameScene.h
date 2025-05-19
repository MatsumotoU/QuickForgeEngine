#pragma once
#include "../../Engine/Base/EngineCore.h"

static inline const int kParticles = 512;

class GameScene {
public:
	GameScene(EngineCore* engineCore);

public:
	void Initialize();
	void Update();
	void Draw();

private:
	EngineCore* engineCore_;
	DirectInputManager* input_;

private:
	float t;
	Model model[kParticles];
	Model skyDome_;
	Transform transform;

	Vector4 color;

	Transform particles[kParticles];

#ifdef _DEBUG
	DebugCamera debugCamera_;
#endif // _DEBUG
	Camera camera;
	
};