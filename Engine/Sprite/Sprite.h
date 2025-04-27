#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <string>
#include "../Math/VerTexData.h"

class DirectXCommon;
class TextureManager;
class ImGuiManager;// 削除予定

class PipelineStateObject;

#include "../Base/DirectX/MaterialResource.h"
#include "../Base/DirectX/WVPResource.h"
#include "../Base/DirectX/DirectionalLightResource.h"

#include "../Base/DirectX/Viewport.h"
#include "../Base/DirectX/ScissorRect.h"

class Sprite {
public:
	Sprite(DirectXCommon* dxCommon, TextureManager* textureManager, ImGuiManager* imguiManager,float width,float hight,PipelineStateObject* pso);
	~Sprite();

public:
	void DrawSprite(int32_t textureHandle, ViewPort* viewport, ScissorRect* scissor);

public:
	MaterialResource material_;
	WVPResource wvp_;
	DirectionalLightResource directionalLight_;

private:
	DirectXCommon* dxCommon_;
	TextureManager* textureManager_;
	ImGuiManager* imGuiManager_;
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_;
	VertexData* vertexData_;
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource_;
	D3D12_INDEX_BUFFER_VIEW indexBufferView_;
	uint32_t* indexData_;

	PipelineStateObject* pso_;
};