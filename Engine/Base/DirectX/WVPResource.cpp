#include "WVPResource.h"
#include "DirectXCommon.h"
#include "../../Math/TransformationMatrix.h"

WVPResource::WVPResource(DirectXCommon* dxCommon) {
	// WVP用のリソースを作る。Matrix4x4一つ分のサイズを用意する
	wvpResource_ = CreateBufferResource(dxCommon->GetDevice(), sizeof(TransformationMatrix));
	// データを書き込む
	wvpData_ = nullptr;
	// 書き込むためのアドレスを取得
	wvpResource_->Map(0, nullptr, reinterpret_cast<void**>(&wvpData_));
	// 単位行列を書き込んでおく
	wvpData_->WVP = Matrix4x4::MakeIndentity4x4();
	wvpData_->World = Matrix4x4::MakeIndentity4x4();
}

ID3D12Resource* WVPResource::GetWVPResource() {
    return wvpResource_.Get();
}
