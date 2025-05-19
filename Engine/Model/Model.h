#pragma once
#include <d3d12.h>
#include <wrl.h>
#include "ModelData.h"
#include "../Math/Transform.h"

#include "../Base/DirectX/MaterialResource.h"
#include "../Base/DirectX/WVPResource.h"
#include "../Base/DirectX/DirectionalLightResource.h"

class EngineCore;
class DirectXCommon;
class TextureManager;
class PipelineStateObject;
class Camera;

class Model {
public:
	void Initialize(EngineCore* engineCore);
	/// <summary>
	/// モデルを読み込みます
	/// </summary>
	/// <param name="directoryPath">directoryPath</param>
	/// <param name="filename">filename</param>
	/// <param name="coordinateSystem">読み込むモデルの座標系</param>
	void LoadModel(const std::string& directoryPath, const std::string& filename, CoordinateSystem coordinateSystem);
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