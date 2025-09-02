#pragma once
#include "Base/EngineCore.h"
#include "IScene.h"

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

#ifdef _DEBUG
	DebugCamera debugCamera_;
	bool isActiveDebugCamera_;
#endif // _DEBUG
};