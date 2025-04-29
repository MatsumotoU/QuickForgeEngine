#include "MaterialResource.h"
#include "DirectXCommon.h"
#include "../../Object/Material.h"

#ifdef _DEBUG
#include "../MyDebugLog.h"
#endif // _DEBUG

void MaterialResource::Initialize(DirectXCommon* dxCommon, const Vector4& color, bool enebleLighting) {
	// マテリアル用のリソースを作る
	materialResource_ = CreateBufferResource(dxCommon->GetDevice(), sizeof(Material));
	// マテリアルにデータを書き込む
	materialData_ = nullptr;
	// 書き込むためのアドレス取得
	materialResource_.Get()->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
	// 今回は赤を書き込んでみる
	materialData_->color = color;
	materialData_->enableLighting = enebleLighting;
	materialData_->uvTransform = Matrix4x4::MakeIndentity4x4();
	DebugLog("CreateMaterialResource");
}

ID3D12Resource* MaterialResource::GetMaterial() {
	return materialResource_.Get();
}

void MaterialResource::SetUvTransformMatrix(const Matrix4x4& uvTransformMatrix) {
	materialData_->uvTransform = uvTransformMatrix;
}
