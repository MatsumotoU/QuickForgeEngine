#include  "DirectXResource.h"

#include "EngineDefines.h"

using namespace QFE::GRAPHIC::INTERNAL;

bool DirectXResource::CreateResource(
	ID3D12Device* device, const D3D12_RESOURCE_DESC& resourceDesc, D3D12_RESOURCE_STATES initialState, D3D12_HEAP_TYPE heapType) {
	// 引数の検査
	if (!device) {
		QFE_REPORT_SYSTEM_ERROR("Device is null in DirectXResource::CreateResource", SystemError::Abort);
		return false;
	}
	if (resourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_UNKNOWN) {
		QFE_REPORT_SYSTEM_ERROR("Resource dimension is unknown in DirectXResource::CreateResource", SystemError::Abort);
		return false;
	}
	if (resourceDesc.Width == 0 || resourceDesc.Height == 0 || resourceDesc.DepthOrArraySize == 0) {
		QFE_REPORT_SYSTEM_ERROR("Resource size is invalid in DirectXResource::CreateResource", SystemError::Abort);
		return false;
	}


	// ヒーププロパティの設定
	D3D12_HEAP_PROPERTIES heapProps{};
	heapProps.Type = heapType;

	// リソースの生成
	HRESULT hr = device->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		initialState,
		nullptr,
		IID_PPV_ARGS(resource_.GetAddressOf())
	);
	if (FAILED(hr)) {
		QFE_REPORT_SYSTEM_ERROR("Failed to create resource in DirectXResource::CreateResource", SystemError::Abort);
		return false;
	}

	// リソースの説明を保存
	resourceDesc_ = resourceDesc;

	// 初期状態を保存
	currentState_ = initialState;
	
	return true;
}

bool DirectXResource::MapResource(UINT subresource, const D3D12_RANGE* readRange) {
	// 引数の検査
	if(readRange == nullptr) {
		QFE_REPORT_SYSTEM_ERROR("Read range is null in DirectXResource::MapResource", SystemError::Abort);
		return false;
	}
	// リソースが生成されているかの確認
	if(resourceDesc_.Dimension == D3D12_RESOURCE_DIMENSION_UNKNOWN) {
		QFE_REPORT_SYSTEM_ERROR("Resource dimension is unknown in DirectXResource::MapResource", SystemError::Abort);
		return false;
	}

	// リソースのマップ
	HRESULT hr = resource_->Map(subresource, readRange, &mappedData_);
	if (FAILED(hr)) {
		QFE_REPORT_SYSTEM_ERROR("Failed to map resource in DirectXResource::MapResource", SystemError::Abort);
		return false;
	}

	return true;
}

bool DirectXResource::UnmapResource() {
	// リソースが生成されているかの確認
	if (resourceDesc_.Dimension == D3D12_RESOURCE_DIMENSION_UNKNOWN) {
		QFE_LOG("Resource dimension is unknown in DirectXResource::UnmapResource");
		return false;
	}

	// リソースのアンマップ
	resource_->Unmap(0, nullptr);
	mappedData_ = nullptr;
	return true;
}

bool DirectXResource::TransitionResource(ID3D12GraphicsCommandList* commandList, D3D12_RESOURCE_STATES newState) {
	// 引数の検査
	if (!commandList) {
		QFE_REPORT_SYSTEM_ERROR("Command list is null in DirectXResource::TransitionResource", SystemError::Abort);
		return false;
	}
	// リソースが生成されているかの確認
	if (resourceDesc_.Dimension == D3D12_RESOURCE_DIMENSION_UNKNOWN) {
		QFE_REPORT_SYSTEM_ERROR("Resource dimension is unknown in DirectXResource::TransitionResource", SystemError::Abort);
		return false;
	}

	// 状態の遷移
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;// 状態遷移のバリア
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;// バリアのフラグ
	barrier.Transition.pResource = resource_.Get();// 遷移するリソース
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;// 遷移するサブリソース
	barrier.Transition.StateBefore = currentState_;// 遷移前の状態
	barrier.Transition.StateAfter = newState;// 遷移後の状態
	commandList->ResourceBarrier(1, &barrier);
	beforeState_ = currentState_;
	currentState_ = newState;

	return true;
}

bool DirectXResource::TransitionResourceToBeforeState(ID3D12GraphicsCommandList* commandList) {
	// 引数の検査
	if (!commandList) {
		QFE_REPORT_SYSTEM_ERROR("Command list is null in DirectXResource::TransitionResourceToBeforeState", SystemError::Abort);
		return false;
	}
	// リソースが生成されているかの確認
	if (resourceDesc_.Dimension == D3D12_RESOURCE_DIMENSION_UNKNOWN) {
		QFE_REPORT_SYSTEM_ERROR("Resource dimension is unknown in DirectXResource::TransitionResourceToBeforeState", SystemError::Abort);
		return false;
	}

	// 状態の遷移
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;// 状態遷移のバリア
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;// バリアのフラグ
	barrier.Transition.pResource = resource_.Get();// 遷移するリソース
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;// 遷移するサブリソース
	barrier.Transition.StateBefore = currentState_;// 遷移前の状態
	barrier.Transition.StateAfter = beforeState_;// 遷移後の状態
	commandList->ResourceBarrier(1, &barrier);
	currentState_ = beforeState_;
	return true;
}

D3D12_GPU_VIRTUAL_ADDRESS DirectXResource::GetGpuVirtualAddress() const {
	// リソースが生成されているかの確認
	if (resourceDesc_.Dimension == D3D12_RESOURCE_DIMENSION_UNKNOWN) {
		QFE_REPORT_SYSTEM_ERROR("Resource dimension is unknown in DirectXResource::GetGpuVirtualAddress",SystemError::Abort);
		return UINT64_MAX;
	}
	return resource_->GetGPUVirtualAddress();
}

ID3D12Resource* DirectXResource::GetResource() {
	// リソースが生成されているかの確認
	if (resourceDesc_.Dimension == D3D12_RESOURCE_DIMENSION_UNKNOWN) {
		QFE_REPORT_SYSTEM_ERROR("Resource dimension is unknown in DirectXResource::GetResource", SystemError::Abort);
		return nullptr;
	}
	return resource_.Get();
}
