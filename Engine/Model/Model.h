#pragma once
#include <d3d12.h>
#include <wrl.h>
#include "ModelData.h"
#include "../Math/Transform.h"

#include "../Base/DirectX/MaterialResource.h"
#include "../Base/DirectX/WVPResource.h"
#include "../Base/DirectX/DirectionalLightResource.h"

class DirectXCommon;
class TextureManager;
class PipelineStateObject;
class Camera;

class Model {
public:
	void Initialize(DirectXCommon* dxCommon, TextureManager* textureManager, PipelineStateObject* pso);
	void LoadModel(const std::string& directoryPath, const std::string& filename);
	void Draw(const Transform& transform, Camera* camera);

public:
	MaterialResource material_;
	DirectionalLightResource directionalLight_;
	WVPResource wvp_;

private:
	DirectXCommon* dxCommon_;
	TextureManager* textureManager_;

private:
	ModelData modelData_;
	VertexData* vertexData_;
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_;
	int32_t modelTextureHandle_;
	PipelineStateObject* pso_;
};