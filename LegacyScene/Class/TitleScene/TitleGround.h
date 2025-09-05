#pragma once
#include "Base/EngineCore.h"

class TitleGround
{
public:
	TitleGround() = default;
	~TitleGround() = default;

	void DebugImGui();

	void Initialize(EngineCore* engineCore, Camera* camera);
	void Update();
	void Draw();

private:
	EngineCore* engineCore_;
	Camera* camera_;
	std::vector <std::vector<std::unique_ptr<Model>>> model_;

	float blockSize = 1.0f;

	const int vertical = 10;
	const int horizontal = 15;

};
