#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <cassert>
#include "Base/DirectX/DirectXCommon.h"
#include "Base/DirectX/Descriptors/SrvDescriptorHeap.h"

template<class T>
class StructuredBuffer final {
public:
	void CreateResource(DirectXCommon* dxcommon, SrvDescriptorHeap* srvDescriptor, uint32_t elementCount) {
		// Resourceの生成
		structSize_ = sizeof(T);
		elementCount_ = elementCount;

		assert(dxcommon);
		assert(structSize_ > 0);
		assert(elementCount > 0);
		D3D12_HEAP_PROPERTIES heapProps = {};
		heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
		heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapProps.CreationNodeMask = 1;
		heapProps.VisibleNodeMask = 1;
		D3D12_RESOURCE_DESC resourceDesc = {};
		resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		resourceDesc.Alignment = 0;
		resourceDesc.Width = structSize_ * elementCount_;
		resourceDesc.Height = 1;
		resourceDesc.DepthOrArraySize = 1;
		resourceDesc.MipLevels = 1;
		resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
		resourceDesc.SampleDesc.Count = 1;
		resourceDesc.SampleDesc.Quality = 0;
		resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
		HRESULT hr = dxcommon->GetDevice()->CreateCommittedResource(
			&heapProps,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&bufferResource_)
		);

		if (FAILED(hr)) {
			bufferResource_ = nullptr;
			data_ = nullptr;
			return;
		}
		bufferResource_->Map(0, nullptr, reinterpret_cast<void**>(&data_));

		// ResourceをSrvに登録
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Buffer.FirstElement = 0;
		srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
		srvDesc.Buffer.NumElements = elementCount;
		srvDesc.Buffer.StructureByteStride = structSize_;

		instancingSrvHandles_ = srvDescriptor->AssignEmptyArrayHandles();
		srvDescriptor->AssignHeap(bufferResource_.Get(), srvDesc, instancingSrvHandles_.cpuHandle_);
	}

public:
	T* GetData() const { return data_; }
	ID3D12Resource* GetResource() const { return bufferResource_.Get(); }
	D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() const {
		return bufferResource_->GetGPUVirtualAddress();
	}
	DescriptorHandles GetInstancingSrvHandles() const { return instancingSrvHandles_; }
	void SetData(const T& data, uint32_t index) {
		assert(data_);
		assert(index < elementCount_);
		data_[index] = data;
	}

private:
	T* data_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> bufferResource_;
	uint32_t structSize_ = 0;
	uint32_t elementCount_ = 0;
	DescriptorHandles instancingSrvHandles_;
};