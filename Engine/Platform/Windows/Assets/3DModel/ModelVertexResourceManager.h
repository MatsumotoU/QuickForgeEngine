#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <vector>
#include <memory>
#include <unordered_map>
#include <string>

#include "Graphic/ShaderBuffer/VertexBuffer.h"

class DirectXCommon;

class ModelVertexResourceManager final {
public:
	void Initialize(DirectXCommon* dxCommon);

	void AssignModelVertexBuffer(const std::string& modelName, const ModelData& modelData);

	// モデルの頂点バッファを取得する
	ID3D12Resource* GetModelVertexBuffer(const std::string& modelName);
	// 頂点バッファビューを取得する
	D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView(const std::string& modelName);

private:
	DirectXCommon* dxCommon_;
	std::unordered_map<std::string, VertexBuffer<VertexData>> modelVertexBuffers;
};