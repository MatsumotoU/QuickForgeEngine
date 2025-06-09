#pragma once
#include <string>
#include <vector>

#include "../../../externals/DirectXTex/d3dx12.h"
#include "../../../externals/DirectXTex/DirectXTex.h"

class DirectXCommon;
class SrvDescriptorHeap;

class TextureManager final {
public:// 一回は絶対に呼び出さないとバグるやつ
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(DirectXCommon* dxCommon, SrvDescriptorHeap* srvDescriptorHeap);
	/// <summary>
	/// 終了処理
	/// </summary>
	void Finalize();

public:// デバッグ機能
#ifdef _DEBUG
	/// <summary>
	/// 現在読み込んでいるテクスチャを表示します
	/// </summary>
	void DrawImages();
#endif // _DEBUG

public:
	/// <summary>
	/// テクスチャを読み込んでミップ付きマップにして読み込みます
	/// </summary>
	/// <param name="filePath"></param>
	/// <returns>ミップ付きマップ</returns>
	DirectX::ScratchImage Load(const std::string& filePath);
	void LoadScratchImage(const std::string& filePath);
	/// <summary>
	/// テクスチャリソースを作成します
	/// </summary>
	/// <param name="metadata"></param>
	/// <returns>テクスチャリソース</returns>
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateTextureResource(const DirectX::TexMetadata& metadata);
	
	void TransitionResourceBarrier(ID3D12Resource* texture, ID3D12GraphicsCommandList* commandList);
	Microsoft::WRL::ComPtr<ID3D12Resource> UploadTextureData(ID3D12Resource* texture, const DirectX::ScratchImage& mipImages, ID3D12GraphicsCommandList* commandList);
	void EndUploadTextureData(ID3D12Resource* texture, ID3D12GraphicsCommandList* commandList);

	void CreateShaderResourceView(const DirectX::TexMetadata& metadata, ID3D12Resource* textureResource, uint32_t index);
	void CreateOffscreenShaderResourceView();
public:
	void PreDraw();
	void PostDraw();
	void ReleaseIntermediateResources();

public:
	int32_t LoadTexture(const std::string& filePath);

public:
	D3D12_CPU_DESCRIPTOR_HANDLE GetTextureSrvHandleCPU(uint32_t index);
	D3D12_GPU_DESCRIPTOR_HANDLE GetTextureSrvHandleGPU(uint32_t index);
	D3D12_GPU_DESCRIPTOR_HANDLE GetOffscreenSrvHandleGPU();

private:// メンバ変数
	ID3D12Device* device_;
	SrvDescriptorHeap* srvDescriptorHeap_;
	D3D12_HEAP_PROPERTIES heapProperties_;
	D3D12_RESOURCE_DESC resourceDesc_;
	uint32_t srvHandleIndex_;
	std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> textureSrvHandleCPU_;
	std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> textureSrvHandleGPU_;
	D3D12_CPU_DESCRIPTOR_HANDLE offscreenSrvHandleCPU_;
	D3D12_GPU_DESCRIPTOR_HANDLE offscreenSrvHandleGPU_;
	DirectXCommon* dxCommon_;

	int32_t textureHandle_;
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> textureResources_;
	std::vector<DirectX::ScratchImage> scratchImages_;
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> intermediateResource_;
	std::vector<std::string> filePathliblary_;
};