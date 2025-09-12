#pragma once
#include "IScene.h"

class SceneObject final: public IScene {
public:
	SceneObject();
	~SceneObject() override;
	void Initialize() override;
	void Update() override;
	void Draw() override;
	IScene* GetNextScene() override;
private:

};