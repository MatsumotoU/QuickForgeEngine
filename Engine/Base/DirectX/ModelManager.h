#pragma once
#include <string>
#include "../../Math/ModelData.h"
#include "../../Math/MaterialData.h"

class ModelManager {
public:

	static ModelData LoadObjFile(const std::string& directoryPath, const std::string& filename);
	static MaterialData LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename);

private:



};