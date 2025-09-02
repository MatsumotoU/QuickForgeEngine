#pragma once
#include <string>
#include <vector>
#include "Core/Math/Vector/Vector2.h"
#include "Externals/DirectXTex/d3dx12.h"
#include "Externals/DirectXTex/DirectXTex.h"
#include "Utility/String/StringLiblary.h"

class SrvDescriptorHeap;

class TextureManager final {
public:// 一回は絶対に呼び出さないとバグるやつ
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, SrvDescriptorHeap* srvDescriptorHeap);
	/// <summary>
	/// 終了処理
	/// </summary>
	void Finalize();
	/// <summary>
	/// 中間リソースを削除します
	/// </summary>
	void ReleaseIntermediateResources();
	/// <summary>
	/// 指定のパスの画像ファイルを読み取ります
	/// </summary>
	/// <param name="filePath"></param>
	/// <returns></returns>
	int32_t LoadTexture(const std::string& filePath);

	Vector2 GetTextureSize(int32_t textureHandle);
	D3D12_CPU_DESCRIPTOR_HANDLE GetTextureSrvHandleCPU(uint32_t index);
	D3D12_GPU_DESCRIPTOR_HANDLE GetTextureSrvHandleGPU(uint32_t index);

private:
	DirectX::ScratchImage Load(const std::string& filePath);
	void LoadScratchImage(const std::string& filePath);
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateTextureResource(const DirectX::TexMetadata& metadata);
	void TransitionResourceBarrier(ID3D12Resource* texture, ID3D12GraphicsCommandList* commandList);
	Microsoft::WRL::ComPtr<ID3D12Resource> UploadTextureData(ID3D12Resource* texture, const DirectX::ScratchImage& mipImages, ID3D12GraphicsCommandList* commandList);
	void EndUploadTextureData(ID3D12Resource* texture, ID3D12GraphicsCommandList* commandList);
	void CreateShaderResourceView(const DirectX::TexMetadata& metadata, ID3D12Resource* textureResource);
	void CreateOffscreenShaderResourceView();	

private:// メンバ変数
	ID3D12Device* device_;
	ID3D12GraphicsCommandList* commandList_;
	SrvDescriptorHeap* srvDescriptorHeap_;
	D3D12_HEAP_PROPERTIES heapProperties_;
	D3D12_RESOURCE_DESC resourceDesc_;
	uint32_t srvHandleIndex_;
	std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> textureSrvHandleCPU_;
	std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> textureSrvHandleGPU_;

	int32_t textureHandle_;
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> textureResources_;
	std::vector<DirectX::ScratchImage> scratchImages_;
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> intermediateResource_;
	StringLiblary filePathLiblary_;

#ifdef _DEBUG
	int debugTextureIndex_;
#endif // _DEBUG
};