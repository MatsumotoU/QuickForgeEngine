#include "DirectionnalLightResource.h"
#include "DirectXCommon.h"
#include "../../Object/DirectionalLight.h"

DirectionnalLightResource::DirectionnalLightResource(DirectXCommon* dxCommon) {
	// 光源用のリソースを作る。DirectionnalLight一つ分のサイズを用意する
	directionnalLightResource_ = CreateBufferResource(dxCommon->GetDevice(), sizeof(DirectionalLight));
	// データを書き込む
	directionnalLightData_ = nullptr;
	// 書き込むためのアドレスを取得
	directionnalLightResource_->Map(0, nullptr, reinterpret_cast<void**>(&directionnalLightData_));
	// 情報を書き込んでおく
	directionnalLightData_->color = { 1.0f,1.0f,1.0f,1.0f };
	directionnalLightData_->direction = { 0.0f,-1.0f,0.0f };
	directionnalLightData_->intensity = 1.0f;
}

ID3D12Resource* DirectionnalLightResource::GetDirectionnalLightResource() {
	return directionnalLightResource_.Get();
}
