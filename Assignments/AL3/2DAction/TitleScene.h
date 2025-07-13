#pragma once
#include "../../../Engine/Base/EngineCore.h"
#include "IScene.h"
#include "Particle/Particle.h"

class TitleScene : public IScene {
public:
	TitleScene(EngineCore* engineCore);
	~TitleScene() override;

	void Initialize() override;
	void Update() override;
	void Draw() override;

	IScene* GetNextScene() override;

private:
	float frameCount_;
	EngineCore* engineCore_;
	float frameCounter_;
	uint32_t uvCheckerTextureHandle_;
	Model model_;
	Transform titleTransform_;
	Camera camera_;
#ifdef _DEBUG
	DebugCamera debugCamera_;
#endif // _DEBUG
};