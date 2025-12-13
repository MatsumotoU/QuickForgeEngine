#pragma once
#include <string>
#include <vector>
#include "engine/include/core/Math/Vector/Vector2.h"
#include "Externals/DirectXTex/d3dx12.h"
#include "Externals/DirectXTex/DirectXTex.h"
#include "engine/include/utility/String/StringLiblary.h"

#include "engine/include/utility/DesignPatterns/Singleton.h"

class SrvDescriptorHeap;

class TextureManager final :public Singleton<TextureManager> {
	friend class Singleton<TextureManager>;
	TextureManager() = default;
	TextureManager(const TextureManager&) = delete;
	TextureManager& operator=(const TextureManager&) = delete;
	TextureManager(TextureManager&&) = delete;
	TextureManager& operator=(TextureManager&&) = delete;

public:// 荳蝗槭・邨ｶ蟇ｾ縺ｫ蜻ｼ縺ｳ蜃ｺ縺輔↑縺・→繝舌げ繧九ｄ縺､
	/// <summary>
	/// 蛻晄悄蛹・
	/// </summary>
	void Initialize(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, SrvDescriptorHeap* srvDescriptorHeap);
	/// <summary>
	/// 邨ゆｺ・・逅・
	/// </summary>
	void Finalize();
	/// <summary>
	/// 荳ｭ髢薙Μ繧ｽ繝ｼ繧ｹ繧貞炎髯､縺励∪縺・
	/// </summary>
	void ReleaseIntermediateResources();
	/// <summary>
	/// 謖・ｮ壹・繝代せ縺ｮ逕ｻ蜒上ヵ繧｡繧､繝ｫ繧定ｪｭ縺ｿ蜿悶ｊ縺ｾ縺・
	/// </summary>
	/// <param name="filePath"></param>
	/// <returns></returns>
	[[nodiscard]] int32_t LoadTexture(const std::string& filePath);

	[[nodiscard]] Vector2 GetTextureSize(int32_t textureHandle);
	[[nodiscard]] const D3D12_CPU_DESCRIPTOR_HANDLE GetTextureSrvHandleCPU(uint32_t index) const;
	[[nodiscard]] const D3D12_GPU_DESCRIPTOR_HANDLE GetTextureSrvHandleGPU(uint32_t index) const;
	[[nodiscard]] const std::vector<D3D12_CPU_DESCRIPTOR_HANDLE>& GetTextureSrvHandleCPUList() const;
	[[nodiscard]] const std::vector<D3D12_GPU_DESCRIPTOR_HANDLE>& GetTextureSrvHandleGPUList() const;

private:
	DirectX::ScratchImage Load(const std::string& filePath);
	void LoadScratchImage(const std::string& filePath);
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateTextureResource(const DirectX::TexMetadata& metadata);
	void TransitionResourceBarrier(ID3D12Resource* texture, ID3D12GraphicsCommandList* commandList);
	Microsoft::WRL::ComPtr<ID3D12Resource> UploadTextureData(ID3D12Resource* texture, const DirectX::ScratchImage& mipImages, ID3D12GraphicsCommandList* commandList);
	void EndUploadTextureData(ID3D12Resource* texture, ID3D12GraphicsCommandList* commandList);
	void CreateShaderResourceView(const DirectX::TexMetadata& metadata, ID3D12Resource* textureResource);

	ID3D12Device* device_;
	ID3D12GraphicsCommandList* commandList_;
	SrvDescriptorHeap* srvDescriptorHeap_;
	D3D12_HEAP_PROPERTIES heapProperties_;
	D3D12_RESOURCE_DESC resourceDesc_;
	std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> textureSrvHandleCPU_;
	std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> textureSrvHandleGPU_;

	int32_t textureHandle_;
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> textureResources_;
	std::vector<DirectX::ScratchImage> scratchImages_;
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> intermediateResource_;
	StringLiblary filePathLiblary_;
};
