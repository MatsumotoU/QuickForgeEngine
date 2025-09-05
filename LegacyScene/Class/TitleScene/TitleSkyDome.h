#pragma once
#include "Base/EngineCore.h"

class TitleSkyDome
{
public:
	TitleSkyDome() = default;
	~TitleSkyDome() = default;

	void DebugImGui();

	void Initialize(EngineCore* engineCore, Camera* camera);
	void Update();
	void Draw();

private:
	EngineCore* engineCore_;
	Camera* camera_;
	std::unique_ptr<Model> model_;

	Vector3 directionalLight{0.0f,0.0f,9.0f};
};

