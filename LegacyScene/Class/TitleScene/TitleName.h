#pragma once
#include "Base/EngineCore.h"

class TitleName
{
public:
	TitleName() = default;
	~TitleName() = default;

	void DebugImGui();

	void Initialize(EngineCore* engineCore, Camera* camera, Vector3 directionalLightDir);
	void Update();
	void Draw();

private:
	EngineCore* engineCore_;
	Camera* camera_;
	std::unique_ptr<Model> model_;

	float time = 0;

	Vector3 directionalLightDir_;
};

