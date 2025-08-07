#pragma once
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <string>
#include <vector>

#include "Object/Structs/ModelData.h"

class AssimpModelLoader {
public:
	AssimpModelLoader() = default;
	~AssimpModelLoader() = default;
	// モデルの読み込み
	void LoadModelData(const std::string& directory, const std::string& filename,ModelData& modelData);
};