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

	float rotetaTime = 20.0f;//一周の時間

	Vector3 directionalLightDir_ = { 0.0f, 0.0f, 1.0f };

};


