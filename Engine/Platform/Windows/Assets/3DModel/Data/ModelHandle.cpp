#include "ModelHandle.h"
#include "Assets/AssetManager.h"

void ModelHandle::Deserialize(const nlohmann::json& json) {
	if (json.contains("modelName") && json["modelName"].is_string()) {
		modelName = json["modelName"].get<std::string>();
		handle = AssetManager::GetInstance()->LoadModel(modelName);
	}
}
