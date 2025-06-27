#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <string>
#include "../Math/VerTexData.h"
#include "../Math/Transform.h"

#include "../Base/DirectX/MaterialResource.h"
#include "../Base/DirectX/WVPResource.h"
#include "../Base/DirectX/DirectionalLightResource.h"

class EngineCore;
class DirectXCommon;
class TextureManager;
class PipelineStateObject;

class Billboard {
public:
	void Initialize(EngineCore* engineCore, float width, float hight);
	void Update(const Vector3& cameraRotate);
	void Draw(int32_t textureHandle, Camera* camera);

public:
	Transform transform_;
	Matrix4x4 worldMatrix_;

private:
	MaterialResource material_;
	DirectionalLightResource directionalLight_;
	WVPResource wvp_;

	EngineCore* engineCore_;
	DirectXCommon* dxCommon_;
	TextureManager* textureManager_;
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_;
	VertexData* vertexData_;
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource_;
	D3D12_INDEX_BUFFER_VIEW indexBufferView_;
	uint32_t* indexData_;

	PipelineStateObject* pso_;
};