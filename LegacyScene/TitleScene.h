#pragma once
#include "Base/EngineCore.h"
#include "IScene.h"
#include "Class/TitleScene/Mole.h"
#include "Class/TitleScene/Wall.h"
#include "Class/TitleScene/TitleName.h"
#include "Class/TitleScene/TitleGround.h"
#include "Class/TitleScene/TitleSkyDome.h"
#include "Class/TitleScene/SignBoard.h"
#include "Class/TitleScene/DiggingEffect.h"

class TitleScene : public IScene {
public:
	TitleScene(EngineCore* engineCore);
	~TitleScene() override;

	void Initialize() override;
	void Update() override;
	void Draw() override;

	IScene* GetNextScene() override;

	void CameraUpdate();

private:
	float frameCount_;
	EngineCore* engineCore_;
	Camera camera_;

	std::unique_ptr<Mole> mole_;
	std::unique_ptr<DiggingEffect> diggingEffect_;
	std::unique_ptr<Wall> wall_; 
	std::unique_ptr<SignBoard> signBoard_;
	std::unique_ptr<TitleName> titleName_;
	std::unique_ptr<TitleGround> titleGround_;
	std::unique_ptr<TitleSkyDome> skyDome_;


#ifdef _DEBUG
	DebugCamera debugCamera_;
	bool isActiveDebugCamera_;
#endif // _DEBUG
};