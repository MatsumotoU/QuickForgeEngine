#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <string>
#include "../Math/VerTexData.h"

class DirectXCommon;
class TextureManager;
class ImGuiManager;// 削除予定
class MaterialResource;
class WVPResource;
class DirectionnalLightResource;

class Sprite {
public:
	Sprite(DirectXCommon* dxCommon, TextureManager* textureManager, ImGuiManager* imguiManager,float width,float hight);
	~Sprite();

public:
	void LoadTexture(const std::string& filePath);
	void DrawSprite();

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

	Microsoft::WRL::ComPtr<ID3D12Resource> textureResource_;

private:
	MaterialResource* material_;
	WVPResource* wvp_;
	DirectionnalLightResource* directionalLight_;
};