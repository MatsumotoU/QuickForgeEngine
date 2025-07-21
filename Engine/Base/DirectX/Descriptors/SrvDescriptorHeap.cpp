#include "SrvDescriptorHeap.h"
#include "../DirectXCommon.h"

#ifdef _DEBUG
#include "../../MyDebugLog.h"
#include "../ImGuiManager.h"
#endif // _DEBUG

#include <cassert>

// TODO: メモリ領域の可視化
// TODO: メモリ選定機能の追加、外部から変更できないようにする

void SrvDescriptorHeap::Initialize(ID3D12Device* device, UINT numDescriptors, bool shaderVisible) {
	srvDescriptorHeapSize_ = numDescriptors;

	device_ = device;
	uint32_t size = (numDescriptors / 3);
	textureBeginIndex_ = 1;
	offscreenBeginIndex_ = size;
	arrayBeginIndex_ = size * 2;
	descriptorSizeSRV_ = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	attayAssignCount_ = 0;

	srvDescriptorHeap_ = CreateDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, numDescriptors, shaderVisible);
	DebugLog(std::format("CreateSRVDescriptorHeap: Create {} HeapSpace", numDescriptors));
}

DescriptorHandles SrvDescriptorHeap::AssignTextureHandles(const uint32_t& index) {
	if (index + textureBeginIndex_ >= offscreenBeginIndex_) {
		assert(false && "textureIndex_ >= offscreenBeginIndex_");
	}

	DescriptorHandles result;
	result.cpuHandle_ = GetCPUDecriptorHandle(srvDescriptorHeap_.Get(), descriptorSizeSRV_, index + textureBeginIndex_);
	result.gpuHandle_ = GetGPUDecriptorHandle(srvDescriptorHeap_.Get(), descriptorSizeSRV_, index + textureBeginIndex_);
	DebugLog(std::format("SRVDescriptorHeap: AssignTextureHandleIndex {} ", index + textureBeginIndex_));
	return result;
}

DescriptorHandles SrvDescriptorHeap::AssignOffscreenHandles(const uint32_t& index) {
	if (index + offscreenBeginIndex_ >= arrayBeginIndex_) {
		assert(false && "offscreenBeginIndex_ >= arrayBeginIndex_");
	}

	DescriptorHandles result;
	result.cpuHandle_ = GetCPUDecriptorHandle(srvDescriptorHeap_.Get(), descriptorSizeSRV_, index + offscreenBeginIndex_);
	result.gpuHandle_ = GetGPUDecriptorHandle(srvDescriptorHeap_.Get(), descriptorSizeSRV_, index + offscreenBeginIndex_);
	DebugLog(std::format("SRVDescriptorHeap: AssignOffscreenHandleIndex {} ", index + offscreenBeginIndex_));
	return result;
}

DescriptorHandles SrvDescriptorHeap::AssignArrayHandles(const uint32_t& index) {
	if (index + arrayBeginIndex_ >= srvDescriptorHeapSize_) {
		assert(false && "offscreenBeginIndex_ >= arrayBeginIndex_");
	}

	DescriptorHandles result;
	result.cpuHandle_ = GetCPUDecriptorHandle(srvDescriptorHeap_.Get(), descriptorSizeSRV_, index + arrayBeginIndex_);
	result.gpuHandle_ = GetGPUDecriptorHandle(srvDescriptorHeap_.Get(), descriptorSizeSRV_, index + arrayBeginIndex_);
	DebugLog(std::format("SRVDescriptorHeap: AssignArrayHandleIndex {} ", index + arrayBeginIndex_));
	attayAssignCount_++;
	return result;
}

DescriptorHandles SrvDescriptorHeap::AssignEmptyArrayHandles() {
	if (attayAssignCount_ + arrayBeginIndex_ >= srvDescriptorHeapSize_) {
		assert(false && "offscreenBeginIndex_ >= arrayBeginIndex_");
	}

	DescriptorHandles result;
	result.cpuHandle_ = GetCPUDecriptorHandle(srvDescriptorHeap_.Get(), descriptorSizeSRV_, attayAssignCount_ + arrayBeginIndex_);
	result.gpuHandle_ = GetGPUDecriptorHandle(srvDescriptorHeap_.Get(), descriptorSizeSRV_, attayAssignCount_ + arrayBeginIndex_);
	DebugLog(std::format("SRVDescriptorHeap: AssignArrayHandleIndex {} ", attayAssignCount_ + arrayBeginIndex_));
	attayAssignCount_++;
	return result;
}

void SrvDescriptorHeap::AssignHeap(ID3D12Resource* resource, const D3D12_SHADER_RESOURCE_VIEW_DESC& srvDesc, const D3D12_CPU_DESCRIPTOR_HANDLE& cpuDescriptorHande) {
	device_->CreateShaderResourceView(resource, &srvDesc, cpuDescriptorHande);
}

#ifdef _DEBUG
void SrvDescriptorHeap::DrawDebugWindow() {
	ImGui::Begin("SrvDescriptorHeap");
	ImGui::End();
}
#endif // _DEBUG

ID3D12DescriptorHeap* SrvDescriptorHeap::GetSrvDescriptorHeap() {
	return srvDescriptorHeap_.Get();
}

uint32_t SrvDescriptorHeap::GetTextureBeginIndex() {
	return textureBeginIndex_;
}

uint32_t SrvDescriptorHeap::GetOffscreenBeginIndex() {
	return offscreenBeginIndex_;
}
