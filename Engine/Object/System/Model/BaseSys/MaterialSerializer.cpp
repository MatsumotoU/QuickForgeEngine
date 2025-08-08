#include "MaterialSerializer.h"
#include "Base/EngineCore.h"

nlohmann::json MaterialSerializer::Serialize(const MaterialComponent& materialComponent) {
	nlohmann::json j;
	j["textureFilePath"] = materialComponent.textureFilePath;
	const Vector4& color = materialComponent.material_.GetData()->color;
	j["Color"] = { color.x, color.y, color.z, color.w };
	j["EnableLighting"] = materialComponent.material_.GetData()->enableLighting;
	return j;
}

void MaterialSerializer::Deserialize(EngineCore* engineCore, MaterialComponent& materialComponent, const nlohmann::json& j) {
	materialComponent.textureFilePath.clear(); // 初期化
	materialComponent.textureHandle = 0; // 初期化
	materialComponent.material_.CreateResource(engineCore->GetDirectXCommon()->GetDevice());
	materialComponent.directionalLight_.CreateResource(engineCore->GetDirectXCommon()->GetDevice());

		if (j.contains("textureFilePath")) {
			materialComponent.textureFilePath = j["textureFilePath"].get<std::string>();
		}
	if (j.contains("Color")) {
		const auto& colorArray = j["Color"];
		if (colorArray.size() == 4) {
			materialComponent.material_.GetData()->color = Vector4(
				colorArray[0].get<float>(),
				colorArray[1].get<float>(),
				colorArray[2].get<float>(),
				colorArray[3].get<float>()
			);
		}
	}
	if (j.contains("EnableLighting")) {
		materialComponent.material_.GetData()->enableLighting = j["EnableLighting"].get<bool>();
	}
}