#pragma once
#include "Base/EngineCore.h"

class Mole
{ 
public:
	Mole() = default;
	~Mole() = default;

	void DebugImGui();

	void Initialize(EngineCore* engineCore, Camera* camera);
	void Update();
	void Draw();

private:
	EngineCore* engineCore_;
	Camera* camera_;
	std::unique_ptr<Model> model_;
};

