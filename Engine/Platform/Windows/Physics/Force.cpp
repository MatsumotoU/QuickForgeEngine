#include "Force.h"

Force::Force() {
	velocity = Vector3(0.0f, 0.0f, 0.0f);
	acceleration = Vector3(0.0f, 0.0f, 0.0f);
	mass = 1.0f;
	friction = 1.0f;
	gravityStrength = 1.0f;
	isGravity = true;
}

void Force::AddForce(const Vector3& force) {
	acceleration += force / mass;
}

nlohmann::json Force::Serialize() const {
	nlohmann::json json;
	json["velocity"] = { velocity.x, velocity.y, velocity.z };
	json["acceleration"] = { acceleration.x, acceleration.y, acceleration.z };
	json["mass"] = mass;
	json["friction"] = friction;
	json["gravityStrength"] = gravityStrength;
	json["isGravity"] = isGravity;
	return json;
}

void Force::Deserialize(const nlohmann::json& json) {
	if (json.contains("velocity") && json["velocity"].is_array() && json["velocity"].size() == 3) {
		velocity.x = json["velocity"][0].get<float>();
		velocity.y = json["velocity"][1].get<float>();
		velocity.z = json["velocity"][2].get<float>();
	}
	if (json.contains("acceleration") && json["acceleration"].is_array() && json["acceleration"].size() == 3) {
		acceleration.x = json["acceleration"][0].get<float>();
		acceleration.y = json["acceleration"][1].get<float>();
		acceleration.z = json["acceleration"][2].get<float>();
	}
	if (json.contains("mass") && json["mass"].is_number()) {
		mass = json["mass"].get<float>();
	}
	if (json.contains("friction") && json["friction"].is_number()) {
		friction = json["friction"].get<float>();
	}
	if (json.contains("gravityStrength") && json["gravityStrength"].is_number()) {
		gravityStrength = json["gravityStrength"].get<float>();
	}
	if (json.contains("isGravity") && json["isGravity"].is_boolean()) {
		isGravity = json["isGravity"].get<bool>();
	}
}

std::string Force::GetTypeName() const {
	return "Force";
}
