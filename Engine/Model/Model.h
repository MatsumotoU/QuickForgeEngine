#pragma once
#include <d3d12.h>
#include <wrl.h>
#include "ModelData.h"
#include "Object/BaseGameObject.h"
#include "../Base/DirectX/PipelineStateObject.h"

#include "Base/DirectX/Resource/ShaderBuffers/ConstantBuffer.h"
#include "Base/DirectX/Resource/ShaderBuffers/VertexBuffer.h"

#include "Math/TransformationMatrix.h"
#include "Object/Material.h"
#include "Object/DirectionalLight.h"

// .USD 色々読めるモデルデータ

class EngineCore;
class DirectXCommon;
class TextureManager;
class Camera;

class Model :public BaseGameObject{
public:
	Model() = delete;
	Model(EngineCore* engineCore);
	~Model() override = default;

public:
	void Init() override;
	void Update() override;
	/// <summary>
	/// モデルを読み込みます
	/// </summary>
	/// <param name="directoryPath">directoryPath</param>
	/// <param name="filename">filename</param>
	/// <param name="coordinateSystem">読み込むモデルの座標系</param>
	void LoadModel(const std::string& directoryPath, const std::string& filename, CoordinateSystem coordinateSystem);
	void Draw(Camera* camera);

#ifdef _DEBUG
	void DrawImGui() override;
#endif // _DEBUG

public:
	void SetBlendmode(BlendMode mode);

private:
	EngineCore* engineCore_;
	DirectXCommon* dxCommon_;
	TextureManager* textureManager_;
	PipelineStateObject* pso_;

private:
	ConstantBuffer<TransformationMatrix> wvp_; // ワールドビュー投影行列
	ConstantBuffer<DirectionalLight> directionalLight_; // 環境光
	ConstantBuffer<Material> material_; // マテリアル

	VertexBuffer<VertexData> vertexBuffer_; // 頂点バッファ

	ModelData modelData_;
	int32_t modelTextureHandle_;
};