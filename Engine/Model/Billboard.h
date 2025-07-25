#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <string>

#include "Object/BaseGameObject.h"

#include "Base/DirectX/Resource/ShaderBuffers/ConstantBuffer.h"
#include "Base/DirectX/Resource/ShaderBuffers/VertexBuffer.h"

#include "../Math/VerTexData.h"
#include "../Math/Transform.h"
#include "Math/TransformationMatrix.h"
#include "Object/Material.h"
#include "Object/DirectionalLight.h"

class EngineCore;
class DirectXCommon;
class TextureManager;
class PipelineStateObject;

class Billboard : public BaseGameObject {
public:
	Billboard() = delete;
	Billboard(EngineCore* engineCore, float width, float hight,uint32_t textureHandle);
	~Billboard() override = default;
public:
	void Init() override;
	void Update() override;
	void Draw(Camera* camera) override;

private:
	EngineCore* engineCore_;
	DirectXCommon* dxCommon_;
	TextureManager* textureManager_;

private:
	Vector2 size_;
	ConstantBuffer<TransformationMatrix> wvp_; // ワールドビュー投影行列
	ConstantBuffer<DirectionalLight> directionalLight_; // 環境光
	ConstantBuffer<Material> material_; // マテリアル
	VertexBuffer<VertexData> vertexBuffer_; // 頂点バッファ

	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource_;
	D3D12_INDEX_BUFFER_VIEW indexBufferView_;
	uint32_t* indexData_;

	uint32_t modelTextureHandle_ = 0; // テクスチャハンドル
	PipelineStateObject* pso_;
};