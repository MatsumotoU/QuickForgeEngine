#pragma once
#include <string>
#include "../../../externals/DirectXTex/DirectXTex.h"
#include <d3d12.h>

class TextureManager {
public:
	// シングルトン
	static TextureManager* GetInstatnce();

public:// 一回は絶対に呼び出さないとバグるやつ
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(ID3D12Device* device);
	/// <summary>
	/// 終了処理
	/// </summary>
	void Finalize();

public:
	/// <summary>
	/// テクスチャを読み込んでミップ付きマップにして読み込みます
	/// </summary>
	/// <param name="filePath"></param>
	/// <returns>ミップ付きマップ</returns>
	DirectX::ScratchImage LoadTexture(const std::string& filePath);
	/// <summary>
	/// テクスチャリソースを作成します
	/// </summary>
	/// <param name="metadata"></param>
	/// <returns>テクスチャリソース</returns>
	ID3D12Resource* CreateTextureResource(const DirectX::TexMetadata& metadata);
	/// <summary>
	/// TextureResourceにデータを転送する
	/// </summary>
	/// <param name="texture"></param>
	/// <param name="mipImages"></param>
	void UploadTextureData(ID3D12Resource* texture, const DirectX::ScratchImage& mipImages);

	void CreateShaderResourceView(const DirectX::TexMetadata& metadata,ID3D12DescriptorHeap* srvDescriptorHeap,ID3D12Resource* textureResource);

public:
	D3D12_CPU_DESCRIPTOR_HANDLE GetTextureSrvHandleCPU();
	D3D12_GPU_DESCRIPTOR_HANDLE GetTextureSrvHandleGPU();

private:// メンバ変数
	ID3D12Device* device_;
	D3D12_HEAP_PROPERTIES heapProperties_;

	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU_;
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU_;

private: // シングルトン用
	TextureManager() = default;
	~TextureManager() = default;
	TextureManager(const TextureManager&) = delete;
	TextureManager& operator=(const TextureManager&) = delete;

};