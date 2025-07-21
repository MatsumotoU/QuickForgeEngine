#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <string>
#include "../Math/Transform.h"

class EngineCore;

class DirectXCommon;
class TextureManager;

class PipelineStateObject;

#include "Base/DirectX/Resource/ShaderBuffers/VertexBuffer.h"
#include "../Base/DirectX/MaterialResource.h"
#include "../Base/DirectX/WVPResource.h"
#include "../Base/DirectX/DirectionalLightResource.h"

class Camera;

class Sprite {
public:
	void Initialize(EngineCore* engineCore, float width, float hight);

public:
	void DrawSprite(const Matrix4x4& worldMatrix, int32_t textureHandle, Camera* camera);
	void DrawSprite(const Transform& transform, int32_t textureHandle,Camera* camera);
	void DrawSprite(const Transform& transform, const Transform& uvTransform, int32_t textureHandle, Camera* camera);
	void DrawSprite(const D3D12_GPU_DESCRIPTOR_HANDLE& gpuHandle);
	void DrawSprite(const Transform& transform, const Transform& uvTransform, const D3D12_GPU_DESCRIPTOR_HANDLE& gpuHandle, Camera* camera);

public:
	void SetPSO(PipelineStateObject* pso);

public:
	Transform transform_;

public:
	MaterialResource material_;
	DirectionalLightResource directionalLight_;
	WVPResource wvp_;	

private:
	EngineCore* engineCore_;
	DirectXCommon* dxCommon_;
	TextureManager* textureManager_;
	VertexBuffer<VertexData> vertexBuffer_;
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource_;
	D3D12_INDEX_BUFFER_VIEW indexBufferView_;
	uint32_t* indexData_;

	PipelineStateObject* pso_;
};