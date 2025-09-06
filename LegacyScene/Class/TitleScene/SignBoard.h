#pragma once
#include "Base/EngineCore.h"

class SignBoard
{
public:
	SignBoard() = default;
	~SignBoard() = default;

	void DebugImGui();

	void Initialize(EngineCore* engineCore, Camera* camera);
	void Update();
	void Draw();

private:
	EngineCore* engineCore_;
	Camera* camera_;
	std::unique_ptr<Model> model_;
};
