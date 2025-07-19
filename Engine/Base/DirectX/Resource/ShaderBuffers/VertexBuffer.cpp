#include "VertexBuffer.h"
#include "Base/DirectX/DirectXCommon.h"

void VertexBuffer::CreateResource(ID3D12Device* device, uint32_t vertexCount) {
	vertexCount_ = vertexCount;

	// Spriteを作る
	vertexResource_ = CreateBufferResource(device, sizeof(VertexData) * vertexCount);
	vertexBufferView_ = {};
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	vertexBufferView_.SizeInBytes = sizeof(VertexData) * vertexCount;
	vertexBufferView_.StrideInBytes = sizeof(VertexData);

	// 頂点データ作成
	vertexData_ = nullptr;
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData_));
	for (uint32_t i = 0; i < vertexCount; ++i) {
		vertexData_[i].position = { 0.0f,0.0f,0.0f,1.0f };
		vertexData_[i].texcoord = { 0.0f,0.0f };
		vertexData_[i].normal = { 0.0f,0.0f,1.0f };
	}
}
