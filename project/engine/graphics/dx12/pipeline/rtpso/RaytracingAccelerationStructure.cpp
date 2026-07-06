#include "RaytracingAccelerationStructure.h"
#include "EngineDefines.h"

using namespace QFE::GRAPHIC;

namespace {
	// BLASの最大数を定義
	constexpr UINT MaxBLASCount = 300;
}

void RaytracingAccelerationStructure::Initialize(ID3D12Device5* device5) {
	// TLASのインスタンスを作成
	tlas_ = std::make_unique<TLAS>();
	if (!tlas_->Create(device5, MaxBLASCount)) {
		QFE_LOG("Failed to create TLAS.");
	}
}

void QFE::GRAPHIC::RaytracingAccelerationStructure::UpdateTLAS(
	ID3D12GraphicsCommandList4* commandList4, const std::vector<RaytracingInstance>& instances) {

	// TLASを構築に使うBLASの結果バッファを取得するための関数を定義
	std::function<ID3D12Resource* (BLASHandle)> getResourceFunc = [this](BLASHandle handle) {
		return this->GetBLASResultBuffer(handle); };
	// TLASの構築
	if (!tlas_->Build(commandList4, instances, getResourceFunc)) {
		QFE_LOG("Failed to build TLAS.");
	}
}

BLASHandle RaytracingAccelerationStructure::CreateBLAS(
	ID3D12Device5* device5, ID3D12GraphicsCommandList4* commandList4,
	const std::vector<QFE::MATH::Vector3>& vertices, const std::string& name) {

	// すでに同じ名前のBLASが存在する場合は、既存のハンドルを返す
	if(blasContainer_.Contains(name)) {
		QFE_LOG("BLAS with name " + name + " already exists.");
		return static_cast<BLASHandle>(blasContainer_.GetHandle(name));
	}

	// BLASのインスタンスを作成
	std::unique_ptr<BLAS> blas = std::make_unique<BLAS>();
	if (!blas->Create(device5, commandList4, vertices)) {
		QFE_LOG("Failed to create BLAS for " + name);
		return BLASHandle::Invalid;
	}
	// BLASをコンテナに追加し、ハンドルを取得
	BLASHandle handle = static_cast<BLASHandle>(blasContainer_.Add(name, std::move(blas)));
	return handle;
}

ID3D12Resource* RaytracingAccelerationStructure::GetBLASResultBuffer(BLASHandle handle) const {
	// BLASハンドルが有効かどうかを確認
	if(handle == BLASHandle::Invalid) {
		QFE_LOG("Invalid BLAS handle.");
		return nullptr;
	}

	// BLASコンテナにハンドルが存在するか確認,ある場合はBLASの結果バッファを返す
	if(blasContainer_.Contains(static_cast<uint32_t>(handle))) {
		BLAS* blas = blasContainer_.GetData(static_cast<uint32_t>(handle))->get();
		return blas->GetBLASResultBuffer();
	}
	return nullptr;
}

ID3D12Resource* RaytracingAccelerationStructure::GetTLASResultBuffer() const {
	// TLASが有効かどうかを確認,ある場合はTLASの結果バッファを返す
	if(tlas_) {
		return tlas_->GetTLASResultBuffer();
	}
	return nullptr;
}