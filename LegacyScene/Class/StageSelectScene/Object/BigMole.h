#pragma once
#include "Base/EngineCore.h"

class BigMole
{
public:
	BigMole() = default;
	~BigMole() = default;

	void DebugImGui();

	void Initialize(EngineCore* engineCore, Camera* camera);
	void Update();
	void Draw();

private:
	EngineCore* engineCore_;
	Camera* camera_;
	std::unique_ptr<Model> model_;
	std::unique_ptr<Model> hangar_;

};

