#pragma once
#include "Base/EngineCore.h"

class Wall
{
public:
	Wall() = default;
	~Wall() = default;

	void DebugImGui();

	void Initialize(EngineCore* engineCore, Camera* camera);
	void Update();
	void Draw();

private:
	EngineCore* engineCore_;
	Camera* camera_;
	std::unique_ptr<Model> model_;
};

