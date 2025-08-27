#pragma once
#include "../../../Engine/Base/EngineCore.h"
#include "IScene.h"

#include "Class/Ice.h"
#include "Class/Azarasi.h"
#include "Class/Mouth.h"

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
	Model mainTitlemodel_;
	Transform mainTitleTransform_;
	Model subTitleModel_;
	Transform subTitleTransform_;
	Camera camera_;
	float cameraShakePower_;
	int transitionFrame_;

	Ice ice_;
	Azarasi azarasi_;
	Vector3 azarasiMoveDir_;
	bool isAzarasiMove_;

	int mainTitleBeatRate_;
	int subTitleBeatRate_;

	uint32_t titleBgmHandle_;
};