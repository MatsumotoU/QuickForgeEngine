#pragma once
#include <d3d12.h>
#include <wrl.h>
#include "Math/VerTexData.h"
#include "Math/PrimitiveVertexData.h"

#include <cassert>

template<class T>
class VertexBuffer {
public:
	void CreateResource(ID3D12Device* device, uint32_t vertexCount) {
		vertexCount_ = vertexCount;

		// Spriteを作る
		vertexResource_ = CreateBufferResource(device, sizeof(T) * vertexCount);
		vertexBufferView_ = {};
		vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
		vertexBufferView_.SizeInBytes = sizeof(T) * vertexCount;
		vertexBufferView_.StrideInBytes = sizeof(T);

		// 頂点データ作成
		vertexData_ = nullptr;
		vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData_));
	};

	// ゲッター
	/// <summary>
	/// 安全でないのでセッターを使うことを推奨します。
	/// </summary>
	/// <returns></returns>
	T* GetData() { return vertexData_; }
	ID3D12Resource* GetResource() const { return vertexResource_.Get(); }
	D3D12_VERTEX_BUFFER_VIEW* GetVertexBufferView() { return &vertexBufferView_; }
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