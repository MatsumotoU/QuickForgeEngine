#include "engine/include/camera/Data/BillboardComponent.h"

QFE::Component::BillboardComponent::BillboardComponent() {
	type_ = BillboardType::AXIAL;
	rotateOffset_ = QFE::Vector3(0.0f, 0.0f, 0.0f);
	isActive_ = true;
}

nlohmann::json QFE::Component::BillboardComponent::Serialize() const {
	nlohmann::json json;
	json["type"] = static_cast<int>(type_);
	json["rotateOffset"] = { rotateOffset_.x, rotateOffset_.y, rotateOffset_.z };
	json["isActive"] = isActive_;
	return json;
}

void QFE::Component::BillboardComponent::Deserialize(const nlohmann::json& json) {
	if (json.contains("type")) {
		type_ = static_cast<BillboardType>(json["type"].get<int>());
	}
	if (json.contains("rotateOffset")) {
		auto offset = json["rotateOffset"];
		if (offset.is_array() && offset.size() == 3) {
			rotateOffset_.x = offset[0].get<float>();
			rotateOffset_.y = offset[1].get<float>();
			rotateOffset_.z = offset[2].get<float>();
		}
	}
	if (json.contains("isActive")) {
		isActive_ = json["isActive"].get<bool>();
	}
}
