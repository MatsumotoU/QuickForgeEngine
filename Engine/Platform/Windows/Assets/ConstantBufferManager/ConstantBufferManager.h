#pragma once
#include "pch.h"
#include "Graphic/ShaderBuffer/ConstantBuffer.h"
#include "Graphic/DirectXCommon/DirectXCommon.h"

template<typename T>
class ConstantBufferManager final {
public:
    void Initialize() { buffers_.clear(); }
    uint32_t CreateBuffer() {
        buffers_.emplace_back();
        buffers_.back().CreateResource(DirectXCommon::GetInstance()->GetDevice());
        return static_cast<uint32_t>(buffers_.size() - 1);
    }
    T* GetBufferData(uint32_t handle) {
        assert(handle < buffers_.size() && "Buffer Handle is Out of Range.");
        return buffers_.at(handle).GetData();
    }
	D3D12_GPU_VIRTUAL_ADDRESS GetBufferAddress(uint32_t handle) {
		assert(handle < buffers_.size() && "Buffer Handle is Out of Range.");
		return buffers_.at(handle).GetGPUVirtualAddress();
	}
    void Finalize() { buffers_.clear(); }

private:
    std::vector<ConstantBuffer<T>> buffers_;
};