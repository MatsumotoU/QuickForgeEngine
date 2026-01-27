#include "engine/include/collider/Data/AABBColliderData.h"

using namespace QFE;

AABBColliderData::AABBColliderData() {
	aabb.center = {0.0f, 0.0f, 0.0f};
	aabb.size = {1.0f, 1.0f, 1.0f};
	isTrigger = false;
	isStatic = false;
}

nlohmann::json AABBColliderData::Serialize() const {
	nlohmann::json json;
	json["isTrigger"] = isTrigger;
	json["isStatic"] = isStatic;
	json["aabb"] = { aabb.center.x, aabb.center.y, aabb.center.z, aabb.size.x, aabb.size.y, aabb.size.z };
	json["colliderLayer"] = colliderLayer;
	json["eventColliderLayer"] = eventColliderLayer;
#ifdef QFE_OPTIMIZE_OFF
	json["isDraw"] = isDraw;
#endif // QFE_OPTIMIZE_OFF

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
		aabb.center.x = json["aabb"][0].get<float>();
		aabb.center.y = json["aabb"][1].get<float>();
		aabb.center.z = json["aabb"][2].get<float>();
		aabb.size.x = json["aabb"][3].get<float>();
		aabb.size.y = json["aabb"][4].get<float>();
		aabb.size.z = json["aabb"][5].get<float>();
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
