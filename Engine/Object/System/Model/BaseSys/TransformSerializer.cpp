#include "TransformSerializer.h"
#include "Base/EngineCore.h"

nlohmann::json TransformSerializer::Serialize(const TransformComponent& transformComponent) {
	nlohmann::json j;
	j["position"] = { transformComponent.transform_.translate.x, transformComponent.transform_.translate.y, transformComponent.transform_.translate.z };
	j["rotation"] = { transformComponent.transform_.rotate.x, transformComponent.transform_.rotate.y, transformComponent.transform_.rotate.z };
	j["scale"] = { transformComponent.transform_.scale.x, transformComponent.transform_.scale.y, transformComponent.transform_.scale.z };
	return j;
}

void TransformSerializer::Deserialize(EngineCore* engineCore, TransformComponent& transformComponent, const nlohmann::json& j) {
	transformComponent.transformationBuffer_.CreateResource(engineCore->GetDirectXCommon()->GetDevice());
	transformComponent.transform_.translate = Vector3(0.0f, 0.0f, 0.0f);
	transformComponent.transform_.rotate = Vector3(0.0f, 0.0f, 0.0f);
	transformComponent.transform_.scale = Vector3(1.0f, 1.0f, 1.0f);
	transformComponent.transformationBuffer_.GetData()->World = Matrix4x4::MakeIndentity4x4();
	transformComponent.transformationBuffer_.GetData()->WVP = Matrix4x4::MakeIndentity4x4();

	if (j.contains("position")) {
		const auto& pos = j["position"];
		if (pos.size() == 3) {
			transformComponent.transform_.translate = Vector3(pos[0].get<float>(), pos[1].get<float>(), pos[2].get<float>());
		}
	}
	if (j.contains("rotation")) {
		const auto& rot = j["rotation"];
		if (rot.size() == 3) {
			transformComponent.transform_.rotate = Vector3(rot[0].get<float>(), rot[1].get<float>(), rot[2].get<float>());
		}
	}
	if (j.contains("scale")) {
		const auto& scale = j["scale"];
		if (scale.size() == 3) {
			transformComponent.transform_.scale = Vector3(scale[0].get<float>(), scale[1].get<float>(), scale[2].get<float>());
		}
	}
}
