#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <string>
#include "../Math/VerTexData.h"
#include "../Math/Transform.h"
class EngineCore;

// TODO: 作りかけのこれを完成させる
class FullScreenSprite {
public:
	void Initialize(EngineCore* engineCore);

public:
	void DrawFullScreenSprite();

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_;
	VertexData* vertexData_;
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource_;
	D3D12_INDEX_BUFFER_VIEW indexBufferView_;
	uint32_t* indexData_;

private:
	EngineCore* engineCore_;
};