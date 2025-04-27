#include "DirectionalLightResource.h"
#include "DirectXCommon.h"
#include "../../Object/DirectionalLight.h"

void DirectionalLightResource::Initialize(DirectXCommon* dxCommon) {
	// 光源用のリソースを作る。DirectionnalLight一つ分のサイズを用意する
	directionalLightResource_ = CreateBufferResource(dxCommon->GetDevice(), sizeof(DirectionalLight));
	// データを書き込む
	directionalLightData_ = nullptr;
	// 書き込むためのアドレスを取得
	directionalLightResource_->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData_));
	// 情報を書き込んでおく
	directionalLightData_->color = { 1.0f,1.0f,1.0f,1.0f };
	directionalLightData_->direction = { 0.0f,-1.0f,0.0f };
	directionalLightData_->intensity = 1.0f;
}

ID3D12Resource* DirectionalLightResource::GetDirectionalLightResource() {
	return directionalLightResource_.Get();
}
