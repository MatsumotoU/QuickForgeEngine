#include "ModelMaterialCreater.h"
#include <cassert>
#include "Base/EngineCore.h"

void ModelMaterialCreater::CreateMaterialComponent(EngineCore* engineCore, ModelMaterialData& modelMaterialData, MaterialComponent& materialComponent) {
	assert(engineCore);
	// モデルデータが空でないことを確認
	if (modelMaterialData.textureFilePath.empty()) {
		assert(false && "Model data is empty. Cannot create resources.");
		return;
	}
	// テクスチャファイルパスを設定
	materialComponent.textureFilePath = modelMaterialData.textureFilePath;
	// マテリアルデータを設定
	materialComponent.material_.CreateResource(engineCore->GetDirectXCommon()->GetDevice());
	materialComponent.material_.GetData()->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	materialComponent.material_.GetData()->enableLighting = true;
	materialComponent.material_.GetData()->uvTransform = Matrix4x4::MakeIndentity4x4();
	materialComponent.textureHandle = engineCore->GetTextureManager()->LoadTexture(modelMaterialData.textureFilePath);

	// ライトデータを設定
	materialComponent.directionalLight_.CreateResource(engineCore->GetDirectXCommon()->GetDevice());
	materialComponent.directionalLight_.GetData()->direction = Vector3(0.0f, -1.0f, 0.0f);
	materialComponent.directionalLight_.GetData()->color = Vector4(1.0f, 1.0f, 1.0f,1.0f);
	materialComponent.directionalLight_.GetData()->intensity = 1.0f;
}
