#include "RaytracingAccelerationStructure.h"
#include "EngineDefines.h"

using namespace QFE::GRAPHIC;

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