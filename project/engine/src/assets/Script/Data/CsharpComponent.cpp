#include "CsharpComponent.h"

nlohmann::json CsharpComponent::Serialize() const {
	nlohmann::json json;
	json["CsharpHandles"] = nlohmann::json::array();
	for (const auto& handle : csharpHandles_) {
		nlohmann::json handleJson;
		handleJson["ClassName"] = handle.className_;
		json["CsharpHandles"].push_back(handleJson);
	}
	return json;
}

void CsharpComponent::Deserialize(const nlohmann::json& json) {
	for (const auto& handleJson : json["CsharpHandles"]) {
		CsharpHandle handle;
		handle.className_ = handleJson["ClassName"].get<std::string>();
		csharpHandles_.push_back(handle);
	}
}
