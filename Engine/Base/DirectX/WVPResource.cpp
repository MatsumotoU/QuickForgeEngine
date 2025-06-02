#include "WVPResource.h"
#include "DirectXCommon.h"
#include "../../Math/TransformationMatrix.h"

void WVPResource::Initialize(DirectXCommon* dxCommon, uint32_t totalResources) {
	// WVP用のリソースを作る。Matrix4x4一つ分のサイズを用意する
	wvpResource_ = CreateBufferResource(dxCommon->GetDevice(), sizeof(TransformationMatrix) * totalResources);
	// データを書き込む
	wvpData_ = nullptr;
	// 書き込むためのアドレスを取得
	wvpResource_->Map(0, nullptr, reinterpret_cast<void**>(&wvpData_));
	// 単位行列を書き込んでおく
	for (uint32_t index = 0; index < totalResources; index++) {
		wvpData_[index].WVP = Matrix4x4::MakeIndentity4x4();
		wvpData_[index].World = Matrix4x4::MakeIndentity4x4();
	}
}

ID3D12Resource* WVPResource::GetWVPResource() {
    return wvpResource_.Get();
}

void WVPResource::SetWorldMatrix(const Matrix4x4& worldMatrix,uint32_t index) {
	wvpData_[index].World = worldMatrix;
}

void WVPResource::SetWVPMatrix(const Matrix4x4& wvpMatrix, uint32_t index) {
	wvpData_[index].WVP = wvpMatrix;
}
