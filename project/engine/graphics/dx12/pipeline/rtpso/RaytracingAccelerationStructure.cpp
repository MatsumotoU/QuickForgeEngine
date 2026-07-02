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

void RaytracingAccelerationStructure::UpdateTLAS(ID3D12GraphicsCommandList4* commandList4) {
	// TLASを構築するためのインスタンス情報を収集
	std::vector<RaytracingInstance> instances;
	for (const auto& instance : instanceSet_) {
		instances.push_back(instance);
	}
	// TLASを構築に使うBLASの結果バッファを取得するための関数を定義
	std::function<ID3D12Resource*(BLASHandle)> getResourceFunc = [this](BLASHandle handle) {
		return this->GetBLASResultBuffer(handle);};
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

BLASInstanceHandle RaytracingAccelerationStructure::CreateBLASInstance(
	BLASHandle handle, const QFE::MATH::Matrix4x4& transform) {

	// BLASハンドルが有効かどうかを確認
	if(handle == BLASHandle::Invalid) {
		QFE_LOG("Invalid BLAS handle.");
		return BLASInstanceHandle::Invalid;
	}
	// BLASコンテナにハンドルが存在するか確認
	if(!blasContainer_.Contains(static_cast<uint32_t>(handle))) {
		QFE_LOG("BLAS handle does not exist.");
		return BLASInstanceHandle::Invalid;
	}
	// 新しいBLASインスタンスを作成し、インスタンスセットに追加
	RaytracingInstance instance;
	instance.blasHandle = handle;
	instance.worldMatrix = transform;
	uint32_t instanceKey = instanceSet_.push_back(instance);
	return static_cast<BLASInstanceHandle>(instanceKey);
}

void RaytracingAccelerationStructure::RemoveBLASInstance(BLASInstanceHandle instanceHandle) {
	// インスタンスハンドルが有効かどうかを確認
	if(instanceSet_.Contains(static_cast<uint32_t>(instanceHandle))) {
		instanceSet_.Remove(static_cast<uint32_t>(instanceHandle));
	} else {
		QFE_LOG("Invalid BLAS instance handle.");
	}
}

void RaytracingAccelerationStructure::UpdateBLASInstanceTransform(BLASInstanceHandle instanceHandle, const QFE::MATH::Matrix4x4& transform) {
	// インスタンスハンドルが有効かどうかを確認
	if(instanceSet_.Contains(static_cast<uint32_t>(instanceHandle))) {
		RaytracingInstance* instance = instanceSet_.Get(static_cast<uint32_t>(instanceHandle));
		instance->worldMatrix = transform;
	} else {
		QFE_LOG("Invalid BLAS instance handle.");
	}
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