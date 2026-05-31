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
		// モデルの読み込み
		static void LoadModelData(
			const std::string& filePath, ModelData& modelData);
	};

}
