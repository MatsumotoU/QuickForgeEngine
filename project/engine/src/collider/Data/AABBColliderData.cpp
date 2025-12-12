#include "AABBColliderData.h"

AABBColliderData::AABBColliderData() {
	aabb = { {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f} };
	isTrigger = false;
	isStatic = false;
}

nlohmann::json AABBColliderData::Serialize() const {
	nlohmann::json json;
	json["isTrigger"] = isTrigger;
	json["isStatic"] = isStatic;
	json["aabb"] = { aabb.min.x, aabb.min.y, aabb.min.z, aabb.max.x, aabb.max.y, aabb.max.z };
	json["colliderLayer"] = colliderLayer;
	json["eventColliderLayer"] = eventColliderLayer;
#ifdef _DEBUG
	json["isDraw"] = isDraw;
#endif // _DEBUG

	return json;
}

void AABBColliderData::Deserialize(const nlohmann::json& json) {
	if (json.contains("isTrigger") && json["isTrigger"].is_boolean()) {
		isTrigger = json["isTrigger"].get<bool>();
	}
	if (json.contains("isStatic") && json["isStatic"].is_boolean()) {
		isStatic = json["isStatic"].get<bool>();
	}
	if (json.contains("aabb") && json["aabb"].is_array() && json["aabb"].size() == 6) {
		aabb.min.x = json["aabb"][0].get<float>();
		aabb.min.y = json["aabb"][1].get<float>();
		aabb.min.z = json["aabb"][2].get<float>();
		aabb.max.x = json["aabb"][3].get<float>();
		aabb.max.y = json["aabb"][4].get<float>();
		aabb.max.z = json["aabb"][5].get<float>();
	}
	if (json.contains("colliderLayer") && json["colliderLayer"].is_number_unsigned()) {
		colliderLayer = json["colliderLayer"].get<uint8_t>();
	}
	if (json.contains("eventColliderLayer") && json["eventColliderLayer"].is_number_unsigned()) {
		eventColliderLayer = json["eventColliderLayer"].get<uint8_t>();
	}
#ifdef _DEBUG
	if (json.contains("isDraw") && json["isDraw"].is_boolean()) {
		isDraw = json["isDraw"].get<bool>();
	}
#endif // _DEBUG
}
