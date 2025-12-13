#pragma once
#include <stdint.h>
#include <string>
#include <vector>
#include <list>
#include <unordered_map>
#include <d3d12.h>
#include <memory>
#include <unordered_map>

#include "Data/ModelData.h"
#include "engine/include/graphic/ShaderBuffer/VertexBuffer.h"

class ModelVertexResourceManager final {
public:
	void Initialize();

	uint32_t Assign(ID3D12Device* device,const ModelData& modelData,const std::string& modelName);
	const uint32_t GetVertexBufferCount(uint32_t handle) const;
	ID3D12Resource* GetModelVertexBuffer(const uint32_t& handle);
	VertexData* GetModelVertexBufferData(const uint32_t& handle);
	const D3D12_VERTEX_BUFFER_VIEW* GetVertexBufferView(const uint32_t& handle);

	void Finalize();

private:
	std::unordered_map<std::string, uint32_t> modelHandleMap_;
	std::vector<ModelData> modelDatas_;
	std::vector<VertexBuffer<VertexData>> modelVertexBuffers_;
};
