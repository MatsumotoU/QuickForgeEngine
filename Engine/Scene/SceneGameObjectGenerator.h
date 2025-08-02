#pragma once
#include <string>

class SceneObject;
class AssetManager;
class SceneGameObjectGenerator;

class SceneGameObjectGenerator {
public:
	SceneGameObjectGenerator() = delete;
	SceneGameObjectGenerator(SceneObject* sceneObj,AssetManager* assetMng);

public:
	void AddAsset(const std::string& assetName);

private:
	SceneObject* sceneObject_;
	AssetManager* assetManager_;
};