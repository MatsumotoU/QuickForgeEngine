#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <cassert>

template<class T>
class ConstantBuffer final{
public:
    /// <summary>
    /// リソースを生成します
    /// </summary>
    void CreateResource(ID3D12Device* device) {
		size_ = sizeof(T);

        assert(device);
		assert(size_ > 0);

        D3D12_HEAP_PROPERTIES heapProps = {};
        heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
        heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        heapProps.CreationNodeMask = 1;
        heapProps.VisibleNodeMask = 1;

        D3D12_RESOURCE_DESC resourceDesc = {};
        resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        resourceDesc.Alignment = 0;
        resourceDesc.Width = size_;
        resourceDesc.Height = 1;
        resourceDesc.DepthOrArraySize = 1;
        resourceDesc.MipLevels = 1;
        resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
        resourceDesc.SampleDesc.Count = 1;
        resourceDesc.SampleDesc.Quality = 0;
        resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

        HRESULT hr = device->CreateCommittedResource(
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
        if (data_) {
            *data_ = T();
        }
    }

    // ゲッター
    T* GetData() const { return data_; }
    ID3D12Resource* GetResource() const { return bufferResource_.Get(); }
	D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() const {
		return bufferResource_.Get()->GetGPUVirtualAddress();
	}

private:
    Microsoft::WRL::ComPtr<ID3D12Resource> bufferResource_;
    T* data_ = nullptr;
    size_t size_;
};