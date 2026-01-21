#include "engine/include/collider/Data/SphereColliderData.h"

SphereColliderData::SphereColliderData() {
	sphere = { {0.0f, 0.0f, 0.0f}, 1.0f };
	isTrigger = false;
	isStatic = false;
	colliderLayer = 0xFF;
	eventColliderLayer = 0xFF;
}

nlohmann::json SphereColliderData::Serialize() const {
	nlohmann::json json;
	json["isTrigger"] = isTrigger;
	json["isStatic"] = isStatic;
	json["sphere"] = { sphere.center.x, sphere.center.y, sphere.center.z, sphere.radius };
	json["colliderLayer"] = colliderLayer;
	json["eventColliderLayer"] = eventColliderLayer;
#ifdef QFE_OPTIMIZE_OFF
	json["isDraw"] = isDraw;
#endif // QFE_OPTIMIZE_OFF

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
	if (json.contains("colliderLayer") && json["colliderLayer"].is_number_unsigned()) {
		colliderLayer = json["colliderLayer"].get<uint8_t>();
	}
	if (json.contains("eventColliderLayer") && json["eventColliderLayer"].is_number_unsigned()) {
		eventColliderLayer = json["eventColliderLayer"].get<uint8_t>();
	}
#ifdef QFE_OPTIMIZE_OFF
	if (json.contains("isDraw") && json["isDraw"].is_boolean()) {
		isDraw = json["isDraw"].get<bool>();
	}
#endif // QFE_OPTIMIZE_OFF

}


