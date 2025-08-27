#pragma once
#include "../../../Engine/Base/EngineCore.h"
#include "IScene.h"

#include "Class/Ice.h"
#include "Class/Azarasi.h"
#include "Class/Mouth.h"
#include "Class/KeyTutorial.h"
#include "Class/Score.h"

class TitleScene : public IScene {
public:
	TitleScene(EngineCore* engineCore,nlohmann::json data);
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

	KeyTutorial keyTutorial_;

	int mainTitleBeatRate_;
	int subTitleBeatRate_;

	uint32_t titleBgmHandle_;
	Score score_;

	nlohmann::json json_;

	uint32_t eatSE_;
	uint32_t shotSE_;
	uint32_t exprotionSE_;

	bool isPlayExprotionSE_;
};