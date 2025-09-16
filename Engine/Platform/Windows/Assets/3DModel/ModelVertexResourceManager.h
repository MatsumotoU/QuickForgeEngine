#pragma once
#include "pch.h"
#include <d3d12.h>
#include <memory>

#include "Data/ModelData.h"
#include "Graphic/ShaderBuffer/VertexBuffer.h"

class ModelVertexResourceManager final {
public:
	void Initialize();

	uint32_t Assign(ID3D12Device* device,const ModelData& modelData);
	const uint32_t GetVertexBufferCount(uint32_t handle) const;
	ID3D12Resource* GetModelVertexBuffer(const uint32_t& handle);
	const D3D12_VERTEX_BUFFER_VIEW* GetVertexBufferView(const uint32_t& handle);

	void Finalize();

private:
	std::vector<ModelData> modelDatas_;
	std::vector<VertexBuffer<VertexData>> modelVertexBuffers_;
};