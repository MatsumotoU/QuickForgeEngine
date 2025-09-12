#pragma once
#include <d3d12.h>
#include <memory>
#include <unordered_map>
#include <string>

#include "Data/ModelData.h"
#include "Graphic/ShaderBuffer/VertexBuffer.h"

class ModelVertexResourceManager final {
public:
	void Initialize();

	void AssignModelVertexBuffer(ID3D12Device* device,const std::string& modelName, const ModelData& modelData);

	// モデルの頂点バッファを取得する
	ID3D12Resource* GetModelVertexBuffer(const std::string& modelName);
	// 頂点バッファビューを取得する
	const D3D12_VERTEX_BUFFER_VIEW* GetVertexBufferView(const std::string& modelName);

private:
	std::unordered_map<std::string, VertexBuffer<VertexData>> modelVertexBuffers;
};