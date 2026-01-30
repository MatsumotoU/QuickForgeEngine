#include "engine/include/assets/3DModel/Data/ModelHandle.h"
#include "engine/include/assets/AssetManager.h"

using namespace QFE;

nlohmann::json ModelHandle::Serialize() const {
	nlohmann::json json;
	json["modelName"] = modelName;

	
	AssetManager* assetManager = AssetManager::GetInstance();
	uint32_t materialHandle =
	assetManager->GetModelRenderData(handle)->meshRenderDataHandles[0].materialHandle;
	Material* material = assetManager->GetGpuBufferPool()->GetConstantBufferData<Material>(materialHandle);
	json["color"] = { material->color.x, material->color.y, material->color.z, material->color.w };

	return json;
}

void ModelHandle::Deserialize(const nlohmann::json& json) {
	if (json.contains("modelName") && json["modelName"].is_string()) {
		modelName = json["modelName"].get<std::string>();
		handle = AssetManager::GetInstance()->LoadModel(modelName);

		if (json.contains("color") && json["color"].is_array() && json["color"].size() == 4) {
			AssetManager* assetManager = AssetManager::GetInstance();
			uint32_t materialHandle =
				assetManager->GetModelRenderData(handle)->meshRenderDataHandles[0].materialHandle;
			Material* material = assetManager->GetGpuBufferPool()->GetConstantBufferData<Material>(materialHandle);
			material->color.x = json["color"][0].get<float>();
			material->color.y = json["color"][1].get<float>();
			material->color.z = json["color"][2].get<float>();
			material->color.w = json["color"][3].get<float>();
		}
	}
}
