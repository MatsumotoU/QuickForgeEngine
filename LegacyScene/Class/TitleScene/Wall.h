#pragma once
#include "Base/EngineCore.h"

class Wall
{
public:
	Wall() = default;
	~Wall() = default;

	void DebugImGui();

	void Initialize(EngineCore* engineCore, Camera* camera,Vector3 directionalLightDir);
	void Update();
	void Draw();

private:
	EngineCore* engineCore_;
	Camera* camera_;
	std::unique_ptr<Model> model_;

	Vector3 directionalLightDir_;
};

