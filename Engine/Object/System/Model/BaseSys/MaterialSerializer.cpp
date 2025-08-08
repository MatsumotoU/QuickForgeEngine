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
	materialComponent.material_.GetData()->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f); // デフォルトカラー
	materialComponent.material_.GetData()->uvTransform = Matrix4x4::MakeIndentity4x4();
	materialComponent.directionalLight_.CreateResource(engineCore->GetDirectXCommon()->GetDevice());
	materialComponent.directionalLight_.GetData()->direction = Vector3(0.0f, -1.0f, 0.0f); // デフォルトのライト方向
	materialComponent.directionalLight_.GetData()->color = Vector4(1.0f, 1.0f, 1.0f,1.0f); // デフォルトのライトカラー
	materialComponent.directionalLight_.GetData()->intensity = 1.0f; // デフォルトのライト強度

	if (j.contains("textureFilePath")) {
		materialComponent.textureFilePath = j["textureFilePath"].get<std::string>();
		materialComponent.textureHandle = engineCore->GetTextureManager()->LoadTexture(materialComponent.textureFilePath);
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
		materialComponent.material_.GetData()->enableLighting = j["EnableLighting"].get<uint32_t>();
	}
}