#pragma once
#include <d3d12.h>
#include <wrl.h>
#include "ModelData.h"
#include "AssimpModelLoader.h"
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
	Model(EngineCore* engineCore,Camera* camera);
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
	void Draw()override;

	/// <summary>
	/// jsonデータでオブジェクトを保存します
	/// </summary>
	/// <returns></returns>
	nlohmann::json Serialize() const override;
	/// <summary>
	/// jsonデータからオブジェクトを復元します
	/// </summary>
	/// <param name="j"></param>
	/// <param name="engineCore"></param>
	/// <param name="camera"></param>
	/// <returns></returns>
	static std::unique_ptr<Model> Deserialize(const nlohmann::json& j, EngineCore* engineCore,Camera* camera);
#ifdef _DEBUG
	void DrawImGui() override;
#endif // _DEBUG

public:
	std::string GetModelFileName() const {
		return modelFileName_;
	}
	void SetBlendmode(BlendMode mode);
	void SetColor(const Vector4& color);

private:
	EngineCore* engineCore_;
	DirectXCommon* dxCommon_;
	TextureManager* textureManager_;
	PipelineStateObject* pso_;

private:
	ConstantBuffer<TransformationMatrix> wvp_; // ワールドビュー投影行列
	ConstantBuffer<DirectionalLight> directionalLight_; // 環境光
	ConstantBuffer<Material> material_; // マテリアル

	std::vector<VertexBuffer<VertexData>> vertexBuffers_; // 各メッシュ用
	std::vector<int32_t> modelTextureHandles_; // 各メッシュ用

	std::string modelFileName_;
	ModelData modelData_;
	static int instanceCount_;
};