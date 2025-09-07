#pragma once
#include "Base/EngineCore.h"

class Mole;

class TitleUI
{
public:
	TitleUI() = default;
	~TitleUI() = default;

	void DebugImGui();

	void Initialize(EngineCore* engineCore, Camera* camera, Vector3 directionalLightDir);
	void Update(Mole* mole);
	void Draw(Mole* mole);

private:
	EngineCore* engineCore_;
	Camera* camera_;
	std::unique_ptr<Model> model_;

	float rotetoMax = 3.14f / 4.0f;
	float roteta_ = 0;
	float speed_ = 3.0f;

	Vector3 directionalLightDir_;
};

