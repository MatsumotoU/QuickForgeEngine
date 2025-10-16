#include "SphereColliderData.h"

SphereColliderData::SphereColliderData() {
	sphere = { {0.0f, 0.0f, 0.0f}, 1.0f };
	isTrigger = false;
	isStatic = false;
}

nlohmann::json SphereColliderData::Serialize() const {
	nlohmann::json json;
	json["isTrigger"] = isTrigger;
	json["isStatic"] = isStatic;
	json["sphere"] = { sphere.center.x, sphere.center.y, sphere.center.z, sphere.radius };
#ifdef _DEBUG
	json["isDraw"] = isDraw;
#endif // _DEBUG

	return json;
}

void SphereColliderData::Deserialize(const nlohmann::json& json) {
	if (json.contains("isTrigger") && json["isTrigger"].is_boolean()) {
		isTrigger = json["isTrigger"].get<bool>();
	}
	if (json.contains("isStatic") && json["isStatic"].is_boolean()) {
		isStatic = json["isStatic"].get<bool>();
	}
	if (json.contains("sphere") && json["sphere"].is_array() && json["sphere"].size() == 4) {
		sphere.center.x = json["sphere"][0].get<float>();
		sphere.center.y = json["sphere"][1].get<float>();
		sphere.center.z = json["sphere"][2].get<float>();
		sphere.radius = json["sphere"][3].get<float>();
	} 

#ifdef _DEBUG
	if (json.contains("isDraw") && json["isDraw"].is_boolean()) {
		isDraw = json["isDraw"].get<bool>();
	}
#endif // _DEBUG

}
