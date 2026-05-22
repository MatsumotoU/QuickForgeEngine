#include "engine/include/assets/Animator/AnimationCompornent.h"

nlohmann::json QFE::AnimationComponent::Serialize() const {
	nlohmann::json json;
	json["clipName"] = clipName;
	return json;
}

void QFE::AnimationComponent::Deserialize(const nlohmann::json& json) {
	if (json.contains("clipName") && json["clipName"].is_string()) {
		clipName = json["clipName"].get<std::string>();
	} else {
		clipName = "NoClip";
	}
	return;
}
