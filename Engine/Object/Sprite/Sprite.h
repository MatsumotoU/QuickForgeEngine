#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <string>

#include "Object/BaseGameObject.h"

#include "Math/TransformationMatrix.h"
#include "Object/Material.h"
#include "Object/DirectionalLight.h"

#include "Base/DirectX/Resource/ShaderBuffers/VertexBuffer.h"
#include "Base/DirectX/Resource/ShaderBuffers/ConstantBuffer.h"

class EngineCore;
class Camera;
class DirectXCommon;
class TextureManager;
class PipelineStateObject;

class Sprite : public BaseGameObject {
public:
	Sprite() = delete;
	Sprite(EngineCore* engineCore, Camera* camera, float width, float hight);
	~Sprite() override = default;

public:
	void Init() override;
	void Update() override;
	void Draw() override;

public:
	nlohmann::json Serialize() const override;
	static std::unique_ptr<Sprite> Deserialize(const nlohmann::json& j, EngineCore* engineCore, Camera* camera);

	void SetTextureHandle(uint32_t textureHandle) {
		textureHandle_ = textureHandle;
	}
#ifdef _DEBUG
	void DrawGizmo(const ImGuizmo::OPERATION& op, const ImGuizmo::MODE& mode, const ImVec2& imageScreenPos, const ImVec2& imageSize) override;
	void DrawImGui() override;
#endif // _DEBUG

public:
	ConstantBuffer<TransformationMatrix> wvp_; // ワールドビュー投影行列
	ConstantBuffer<DirectionalLight> directionalLight_; // 環境光
	ConstantBuffer<Material> material_; // マテリアル

private:
	EngineCore* engineCore_;
	DirectXCommon* dxCommon_;
	TextureManager* textureManager_;
	PipelineStateObject* pso_;

	VertexBuffer<VertexData> vertexBuffer_;
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource_;
	D3D12_INDEX_BUFFER_VIEW indexBufferView_;
	uint32_t* indexData_;

	float width_;
	float hight_;
	uint32_t textureHandle_ = 0;
};