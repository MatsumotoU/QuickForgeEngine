#pragma once
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <string>
#include <vector>

#include "ModelData.h"

namespace QFE::GRAPHIC::INTERNAL {

	class AssimpModelLoader {
	public:
		AssimpModelLoader() = default;
		~AssimpModelLoader() = default;
		// モチEの読み込み
		static void LoadModelData(
			const std::string& modelResourceDirectory, const std::string& imageResourceDirectory, const std::string& filename, ModelData& modelData);
	};

}
