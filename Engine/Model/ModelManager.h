#pragma once
#include <string>
#include "ModelData.h"
#include "../Math/MaterialData.h"

class ModelManager {
public:

private:

};

namespace Modelmanager {
	/// <summary>
	/// モデルのデータを読み込みます
	/// </summary>
	/// <param name="directoryPath"></param>
	/// <param name="filename"></param>
	/// <param name="coordinateSystem">"読み込むモデルの"座標系(なってほしい座標系ではない)</param>
	/// <returns></returns>
	ModelData LoadObjFile(const std::string& directoryPath, const std::string& filename, CoordinateSystem coordinateSystem);
	MaterialData LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename);
}