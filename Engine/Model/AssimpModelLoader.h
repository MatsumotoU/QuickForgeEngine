#pragma once
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <string>
#include <vector>

#include "Model/ModelData.h"

class AssimpModelLoader {
public:
	AssimpModelLoader() = default;
	~AssimpModelLoader() = default;
	// モデルの読み込み
	static ModelData LoadModelData(const std::string& directory, const std::string& filename, CoordinateSystem coordinateSystem = COORDINATESYSTEM_HAND_RIGHT);
};