#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <cassert>
#include "Data/VerTexData.h"
#include "Data/PrimitiveVertexData.h"
#include "BufferGenerater/BufferGenerator.h"

template<class T>
class VertexBuffer {
public:
	void CreateResource(ID3D12Device* device, uint32_t vertexCount) {
		assert(device);
		assert(vertexCount != 0);
		vertexCount_ = vertexCount;

		// Spriteを作る
		vertexResource_ = BufferGenerator::Generate(device, sizeof(T) * vertexCount);
		vertexBufferView_ = {};
		vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
		vertexBufferView_.SizeInBytes = sizeof(T) * vertexCount;
		vertexBufferView_.StrideInBytes = sizeof(T);

		// 頂点データ作成
		vertexData_ = nullptr;
		vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData_));
	};

	T* GetData() { return vertexData_; }
	ID3D12Resource* GetResource() const { return vertexResource_.Get(); }
	const D3D12_VERTEX_BUFFER_VIEW* GetVertexBufferView() const { return &vertexBufferView_; }
	D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() const {
		return vertexResource_.Get()->GetGPUVirtualAddress();
	}

	// セッター
	void SetData(uint32_t index, const VertexData& data) {
		assert(index < vertexCount_ && "Index out of bounds");
		vertexData_[index] = data;
	}

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_;
	T* vertexData_;

	uint32_t vertexCount_ = 0;
};