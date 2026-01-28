#pragma once
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <string>
#include <vector>

#include "../Data/ModelData.h"

namespace QFE {

	class AssimpModelLoader {
	public:
		AssimpModelLoader() = default;
		~AssimpModelLoader() = default;
		// モチEの読み込み
		static void LoadModelData(
			const std::string& modelResourceDirectory, const std::string& imageResourceDirectory, const std::string& filename, ModelData& modelData);
	};

}
