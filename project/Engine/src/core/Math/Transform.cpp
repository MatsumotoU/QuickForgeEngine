#include "engine/include/core/Math/Transform.h"

void Transform::FromMatrix(const Matrix4x4& mat) {
    // 平行移動
    translate.x = mat.m[3][0];
    translate.y = mat.m[3][1];
    translate.z = mat.m[3][2];

    // スケール
    scale.x = std::sqrt(mat.m[0][0] * mat.m[0][0] + mat.m[0][1] * mat.m[0][1] + mat.m[0][2] * mat.m[0][2]);
    scale.y = std::sqrt(mat.m[1][0] * mat.m[1][0] + mat.m[1][1] * mat.m[1][1] + mat.m[1][2] * mat.m[1][2]);
    scale.z = std::sqrt(mat.m[2][0] * mat.m[2][0] + mat.m[2][1] * mat.m[2][1] + mat.m[2][2] * mat.m[2][2]);

    // 回転（スケールを除去した3x3部分からXYZオイラー角を取得）
    Matrix4x4 rotMat;
    for (int i = 0; i < 3; ++i) {
        rotMat.m[0][i] = mat.m[0][i] / scale.x;
        rotMat.m[1][i] = mat.m[1][i] / scale.y;
        rotMat.m[2][i] = mat.m[2][i] / scale.z;
    }
    rotMat.m[3][0] = rotMat.m[3][1] = rotMat.m[3][2] = 0.0f;
    rotMat.m[0][3] = rotMat.m[1][3] = rotMat.m[2][3] = 0.0f;
    rotMat.m[3][3] = 1.0f;

    // XYZ順オイラー角
    rotate.y = std::asin(-rotMat.m[2][0]);
    if (std::cos(rotate.y) > 1e-6) {
        rotate.x = std::atan2(rotMat.m[2][1], rotMat.m[2][2]);
        rotate.z = std::atan2(rotMat.m[1][0], rotMat.m[0][0]);
    } else {
        rotate.x = std::atan2(-rotMat.m[1][2], rotMat.m[1][1]);
        rotate.z = 0.0f;
    }
}

nlohmann::json Transform::Serialize() const {
    nlohmann::json json;
    json["scale"] = { scale.x, scale.y, scale.z };
    json["rotate"] = { rotate.x, rotate.y, rotate.z };
    json["translate"] = { translate.x, translate.y, translate.z };
    return json;
}

void Transform::Deserialize(const nlohmann::json& json) {
    if (json.contains("scale") && json["scale"].is_array() && json["scale"].size() == 3) {
        scale.x = json["scale"][0].get<float>();
        scale.y = json["scale"][1].get<float>();
        scale.z = json["scale"][2].get<float>();
    }
    if (json.contains("rotate") && json["rotate"].is_array() && json["rotate"].size() == 3) {
        rotate.x = json["rotate"][0].get<float>();
        rotate.y = json["rotate"][1].get<float>();
        rotate.z = json["rotate"][2].get<float>();
    }
    if (json.contains("translate") && json["translate"].is_array() && json["translate"].size() == 3) {
        translate.x = json["translate"][0].get<float>();
        translate.y = json["translate"][1].get<float>();
        translate.z = json["translate"][2].get<float>();
    }
}

void Transform::AddForward(float distance)  {
	// ラジアンに変換
	float pitch = rotate.x;
	float yaw = rotate.y;
	// 前方ベクトルを計算
	Vector3 forward;
	forward.x = std::cos(pitch) * std::sin(yaw);
	forward.y = std::sin(pitch);
	forward.z = std::cos(pitch) * std::cos(yaw);
	// 位置を更新
	translate.x += forward.x * distance;
	translate.y += forward.y * distance;
	translate.z += forward.z * distance;
}

void Transform::AddRight(float distance) {
	// ラジアンに変換
	float yaw = rotate.y;
	// 右方向ベクトルを計算
	Vector3 right;
	right.x = std::sin(yaw - 3.14f * 0.5f);
	right.y = 0;
	right.z = std::cos(yaw - 3.14f * 0.5f);
	// 位置を更新
	translate.x += right.x * distance;
	translate.y += right.y * distance;
	translate.z += right.z * distance;
}
