#include "engine/include/core/Math/TransformComponent.h"

using namespace QFE;

void TransformComponent::FromMatrix(const Matrix4x4& mat) {
	transform.FromMatrix(mat);
}

nlohmann::json TransformComponent::Serialize() const
{
	nlohmann::json json;
	json["scale"] = { transform.scale.x, transform.scale.y, transform.scale.z };
	json["rotate"] = { transform.rotate.x, transform.rotate.y, transform.rotate.z };
	json["translate"] = { transform.translate.x, transform.translate.y, transform.translate.z };
	return json;
}

void TransformComponent::Deserialize(const nlohmann::json& json) {
    if (json.contains("scale") && json["scale"].is_array() && json["scale"].size() == 3) {
        transform.scale.x = json["scale"][0].get<float>();
        transform.scale.y = json["scale"][1].get<float>();
        transform.scale.z = json["scale"][2].get<float>();
    }
    if (json.contains("rotate") && json["rotate"].is_array() && json["rotate"].size() == 3) {
        transform.rotate.x = json["rotate"][0].get<float>();
        transform.rotate.y = json["rotate"][1].get<float>();
        transform.rotate.z = json["rotate"][2].get<float>();
    }
    if (json.contains("translate") && json["translate"].is_array() && json["translate"].size() == 3) {
        transform.translate.x = json["translate"][0].get<float>();
        transform.translate.y = json["translate"][1].get<float>();
        transform.translate.z = json["translate"][2].get<float>();
    }
}

void TransformComponent::AddForward(float distance)  {
	// ラジアンに変換
	float pitch = transform.rotate.x;
	float yaw = transform.rotate.y;
	// 前方ベクトルを計算
	Vector3 forward;
	forward.x = std::cos(pitch) * std::sin(yaw);
	forward.y = std::sin(pitch);
	forward.z = std::cos(pitch) * std::cos(yaw);
	// 位置を更新
	transform.translate.x += forward.x * distance;
	transform.translate.y += forward.y * distance;
	transform.translate.z += forward.z * distance;
}

void TransformComponent::AddRight(float distance) {
	// ラジアンに変換
	float yaw = transform.rotate.y;
	// 右方向ベクトルを計算
	Vector3 right;
	right.x = std::sin(yaw - 3.14f * 0.5f);
	right.y = 0;
	right.z = std::cos(yaw - 3.14f * 0.5f);
	// 位置を更新
	transform.translate.x += right.x * distance;
	transform.translate.y += right.y * distance;
	transform.translate.z += right.z * distance;
}
