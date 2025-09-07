#pragma once
#include "Base/EngineCore.h"

class SignBoard
{
public:
	SignBoard() = default;
	~SignBoard() = default;

	void DebugImGui();

	void Initialize(EngineCore* engineCore, Camera* camera, Vector3 directionalLightDir);
	void Update();
	void Draw();

private:
	EngineCore* engineCore_;
	Camera* camera_;
	std::unique_ptr<Model> model_;
	std::unique_ptr<Model> letterModel_;

	Vector3 directionalLightDir_;
};
