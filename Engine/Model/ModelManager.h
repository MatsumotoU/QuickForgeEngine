#pragma once
#include <string>
#include "ModelData.h"
#include "../Math/MaterialData.h"

class ModelManager {
public:

private:

};

namespace Modelmanager {
	ModelData LoadObjFile(const std::string& directoryPath, const std::string& filename);
	MaterialData LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename);
}