#include "WVPResource.h"
#include "DirectXCommon.h"
#include "../../Math/TransformationMatrix.h"

void WVPResource::Initialize(DirectXCommon* dxCommon, uint32_t totalResources, bool isParticle) {
	// データを書き込む
	wvpData_ = nullptr;
	particleData_ = nullptr;
	isParticle_ = isParticle;
	
	if (isParticle) {
		// WVP用のリソースを作る。Matrix4x4一つ分のサイズを用意する
		wvpResource_ = CreateBufferResource(dxCommon->GetDevice(), sizeof(ParticleForGPU) * totalResources);
		// 書き込むためのアドレスを取得
		wvpResource_->Map(0, nullptr, reinterpret_cast<void**>(&particleData_));
		// 単位行列を書き込んでおく
		for (uint32_t index = 0; index < totalResources; index++) {
			particleData_[index].WVP = Matrix4x4::MakeIndentity4x4();
			particleData_[index].World = Matrix4x4::MakeIndentity4x4();
			particleData_[index].color = {1.0f,1.0f,1.0f,1.0f};
		}
	} else {
		// WVP用のリソースを作る。Matrix4x4一つ分のサイズを用意する
		wvpResource_ = CreateBufferResource(dxCommon->GetDevice(), sizeof(TransformationMatrix) * totalResources);
		// 書き込むためのアドレスを取得
		wvpResource_->Map(0, nullptr, reinterpret_cast<void**>(&wvpData_));
		// 単位行列を書き込んでおく
		for (uint32_t index = 0; index < totalResources; index++) {
			wvpData_[index].WVP = Matrix4x4::MakeIndentity4x4();
			wvpData_[index].World = Matrix4x4::MakeIndentity4x4();
		}
	}
	
}

ID3D12Resource* WVPResource::GetWVPResource() {
    return wvpResource_.Get();
}

void WVPResource::SetWorldMatrix(const Matrix4x4& worldMatrix,uint32_t index) {
	if (isParticle_) {
		particleData_[index].World = worldMatrix;
	} else {
		wvpData_[index].World = worldMatrix;
	}
	
}

void WVPResource::SetWVPMatrix(const Matrix4x4& wvpMatrix, uint32_t index) {
	if (isParticle_) {
		particleData_[index].WVP = wvpMatrix;
	} else {
		wvpData_[index].WVP = wvpMatrix;
	}
}
