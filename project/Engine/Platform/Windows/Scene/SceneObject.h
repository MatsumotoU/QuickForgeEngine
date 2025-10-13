#pragma once
#include "IScene.h"
class AssetManager;

class SceneObject final: public IScene {
public:
	SceneObject();
	~SceneObject() override;
	void Initialize() override;
	void Update() override;
	void Draw() override;

private:
	AssetManager* assetManager_;
	
};