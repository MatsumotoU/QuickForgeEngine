#pragma once
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <string>
#include <vector>

#include "Object/Data/ModelData.h"

class EngineCore;

class AssimpModelLoader {
public:
	AssimpModelLoader() = default;
	~AssimpModelLoader() = default;
	// モデルの読み込み
	void LoadModelData(EngineCore* engineCore, const std::string& filename,ModelData& modelData);
};