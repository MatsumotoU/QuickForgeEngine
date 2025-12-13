#include "engine/include/assets/2DTexture/TextureManager.h"
#include <cassert>

#include "engine/include/utility/String/MyString.h"
#include "engine/include/Graphic/DirectXCommon/DirectXCommon.h"
#include "engine/include/Graphic/DirectXCommon/Descriptors/SrvDescriptorHeap.h"
#include "engine/include/Graphic/ShaderBuffer/BufferGenerater/BufferGenerator.h"

#ifdef _DEBUG
#include "engine/include/utility/DebugTool/DebugLog/MyDebugLog.h"
#include "engine/include/utility/DebugTool/ImGui/ImGuiInclude.h"
#endif // _DEBUG

void TextureManager::Initialize(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, SrvDescriptorHeap* srvDescriptorHeap) {
	srvDescriptorHeap_ = srvDescriptorHeap;

	// Com縺ｮ蛻晄悄蛹・
	HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);
	assert(SUCCEEDED(hr));
	hr;

	// 繝・ヰ繧､繧ｹ繧貞叙蠕・
	assert(device);
	device_ = device;
	assert(commandList);
	commandList_ = commandList;

	// 蛻ｩ逕ｨ縺吶ｋHeap縺ｮ險ｭ螳・
	heapProperties_ = {};
	heapProperties_.Type = D3D12_HEAP_TYPE_DEFAULT;

	textureSrvHandleCPU_.clear();
	textureSrvHandleGPU_.clear();
	textureResources_.clear();
	scratchImages_.clear();
	intermediateResource_.clear();
	filePathLiblary_.Init("TextureFileName");

	textureHandle_ = 0;
}

void TextureManager::Finalize() {
	// 繝ｪ繧ｽ繝ｼ繧ｹ縺ｮ隗｣謾ｾ
	textureSrvHandleCPU_.clear();
	textureSrvHandleGPU_.clear();
	textureResources_.clear();
	scratchImages_.clear();
	intermediateResource_.clear();

	CoUninitialize();
}

DirectX::ScratchImage TextureManager::Load(const std::string& filePath) {
	// 繝・け繧ｹ繝√Ε繝輔ぃ繧､繝ｫ繧定ｪｭ縺ｿ霎ｼ繧薙〒繝励Ο繧ｰ繝ｩ繝縺ｧ菴ｿ縺医ｋ繧医≧縺ｫ縺吶ｋ
	DirectX::ScratchImage image{};
	std::wstring filePathW = ConvertString(filePath);
	HRESULT hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
	assert(SUCCEEDED(hr));

	// 繝溘ャ繝励・繝・・縺ｮ菴懈・
	DirectX::ScratchImage mipImages{};
	hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImages);
	assert(SUCCEEDED(hr));

	// 繝溘ャ繝嶺ｻ倥″縺ｮ繝・・繧ｿ繧定ｿ斐☆
	return mipImages;
}

void TextureManager::LoadScratchImage(const std::string& filePath) {
	// 繝・け繧ｹ繝√Ε繝輔ぃ繧､繝ｫ繧定ｪｭ縺ｿ霎ｼ繧薙〒繝励Ο繧ｰ繝ｩ繝縺ｧ菴ｿ縺医ｋ繧医≧縺ｫ縺吶ｋ
	DirectX::ScratchImage image{};
	std::wstring filePathW = ConvertString(filePath);
	HRESULT hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
	assert(SUCCEEDED(hr));

#ifdef _DEBUG
	const DirectX::TexMetadata& metadata = image.GetMetadata();
	DebugLog(ConvertString(std::format(L"TextureManager: whidth={},height={},arraySize={}", metadata.width, metadata.height, metadata.arraySize)));
#endif // _DEBUG

	// 繝溘ャ繝励・繝・・縺ｮ菴懈・
	if (image.GetMetadata().width * image.GetMetadata().height != 1) {
		scratchImages_.emplace_back();
		hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, scratchImages_.back());
		assert(SUCCEEDED(hr));
	} else {
		// 縺昴・縺ｾ縺ｾ譬ｼ邏・
		scratchImages_.push_back(std::move(image));
	}
}

Microsoft::WRL::ComPtr<ID3D12Resource> TextureManager::CreateTextureResource(const DirectX::TexMetadata& metadata) {
	// metadata繧貞渕縺ｫResource縺ｮ險ｭ螳・
	resourceDesc_ = {};
	resourceDesc_.Width = static_cast<UINT>(metadata.width); // 繝・け繧ｹ繝√Ε縺ｮ蟷・
	resourceDesc_.Height = static_cast<UINT>(metadata.height); // 繝・け繧ｹ繝√Ε縺ｮ鬮倥＆
	resourceDesc_.MipLevels = static_cast<UINT16>(metadata.mipLevels); // mipmap縺ｮ謨ｰ
	resourceDesc_.DepthOrArraySize = static_cast<UINT16>(metadata.arraySize); // 螂･陦経r驟榊・繝・け繧ｹ繝√Ε縺ｮ驟榊・謨ｰ
	resourceDesc_.Format = metadata.format;
	resourceDesc_.SampleDesc.Count = 1;
	resourceDesc_.Dimension = D3D12_RESOURCE_DIMENSION(metadata.dimension);

	// 繝ｪ繧ｽ繝ｼ繧ｹ縺ｮ逕滓・
	Microsoft::WRL::ComPtr<ID3D12Resource> resource = nullptr;
	HRESULT hr = device_->CreateCommittedResource(
		&heapProperties_,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc_,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(resource.GetAddressOf()));
	assert(SUCCEEDED(hr));
	hr;
	return resource;
}

void TextureManager::TransitionResourceBarrier(ID3D12Resource* texture, ID3D12GraphicsCommandList* commandList) {
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = texture;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_GENERIC_READ;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
	commandList->ResourceBarrier(1, &barrier);
}

Microsoft::WRL::ComPtr<ID3D12Resource> TextureManager::UploadTextureData(ID3D12Resource* texture, const DirectX::ScratchImage& mipImages, ID3D12GraphicsCommandList* commandList) {
	std::vector<D3D12_SUBRESOURCE_DATA> subresources;
	subresources.clear();
	DirectX::PrepareUpload(device_, mipImages.GetImages(), mipImages.GetImageCount(), mipImages.GetMetadata(), subresources);

	UINT64 uploadBufferSize = GetRequiredIntermediateSize(texture, 0, static_cast<UINT>(subresources.size()));
	Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource = BufferGenerator::Generate(device_, uploadBufferSize);

	UpdateSubresources(commandList, texture, intermediateResource.Get(), 0, 0, static_cast<UINT>(subresources.size()), subresources.data());

	EndUploadTextureData(texture, commandList);
	return intermediateResource;
}

void TextureManager::EndUploadTextureData(ID3D12Resource* texture, ID3D12GraphicsCommandList* commandList) {
	// Texture縺ｸ縺ｮ霆｢騾∝ｾ後・蛻ｩ逕ｨ縺ｧ縺阪ｋ繧医≧縺ｫD3D12_RESOURCE_STATE_COPY_DEST縺九ｉD3D12_RESOURCE_STATE_GENERIC_READ縺ｸResouceState繧貞､画峩縺吶ｋ
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = texture;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ;
	commandList->ResourceBarrier(1, &barrier);
}

void TextureManager::CreateShaderResourceView(const DirectX::TexMetadata& metadata, ID3D12Resource* textureResource) {
	// metaData繧貞渕縺ｫSRV縺ｮ險ｭ螳・
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;// 2D繝・け繧ｹ繝√Ε
	srvDesc.Texture2D.MipLevels = static_cast<UINT>(metadata.mipLevels);

	// SRV縺ｮ菴懈・
	DescriptorHandles handles = srvDescriptorHeap_->AssignHeap(textureResource, srvDesc);
	textureSrvHandleCPU_.push_back(handles.cpuHandle_);
	textureSrvHandleGPU_.push_back(handles.gpuHandle_);
}

void TextureManager::ReleaseIntermediateResources() {
	for (int32_t i = 0; i < intermediateResource_.size(); i++) {
		intermediateResource_[i].Reset();
	}
	intermediateResource_.clear();
}

int32_t TextureManager::LoadTexture(const std::string& filePath) {
	// 繝輔ぃ繧､繝ｫ繝代せ陦ｨ遉ｺ
#ifdef _DEBUG
	DebugLog(std::format("TextureManager: LoadPath {}", filePath));
#endif // _DEBUG

	// 蜷後§逕ｻ蜒上ヵ繧｡繧､繝ｫ繧定ｪｭ縺ｿ霎ｼ縺ｾ縺ｪ縺・
	int32_t fileIndex = filePathLiblary_.GetLiblaryIndex(filePath);
	if (fileIndex >= 0) {
#ifdef _DEBUG
		DebugLog(ConvertString(std::format(L"TextureManager: LoadedTheSameFile->return {}", fileIndex)));
#endif // _DEBUG
		return fileIndex;
	}

	// 逕ｻ蜒剰ｪｭ縺ｿ霎ｼ縺ｿ蜃ｦ逅・
	LoadScratchImage(filePath);
	const DirectX::TexMetadata& metadata = scratchImages_.back().GetMetadata();
	textureResources_.emplace_back() = CreateTextureResource(metadata);
#ifdef _DEBUG
	const auto& resource = textureResources_.back();
	if (resource) {
		D3D12_RESOURCE_DESC desc = resource->GetDesc();
		D3D12_RESOURCE_ALLOCATION_INFO allocInfo = device_->GetResourceAllocationInfo(0, 1, &desc);
		DebugLog(std::format("ResourceSize: {}byte", allocInfo.SizeInBytes));
	}
#endif // _DEBUG
	CreateShaderResourceView(metadata, textureResources_.back().Get());
	textureHandle_++;
	intermediateResource_.push_back(
		UploadTextureData(textureResources_.back().Get(), scratchImages_.back(), commandList_));
#ifdef _DEBUG
	DebugLog(ConvertString(std::format(L"TextureManager: whidth={},height={},return->{}", metadata.width, metadata.height, textureHandle_ - 1)));
#endif // _DEBUG
	filePathLiblary_.AddStringToLiblary(filePath);
	return textureHandle_ - 1;
}

Vector2 TextureManager::GetTextureSize(int32_t textureHandle) {
	if (textureHandle < 0 || textureHandle >= static_cast<int32_t>(textureResources_.size())) {
#ifdef _DEBUG
		DebugLog("TextureManager: Invalid texture handle");
#endif // _DEBUG
		return Vector2(0.0f, 0.0f);
	}
	const DirectX::TexMetadata& metadata = scratchImages_[textureHandle].GetMetadata();
	return Vector2(static_cast<float>(metadata.width), static_cast<float>(metadata.height));
}

const D3D12_CPU_DESCRIPTOR_HANDLE TextureManager::GetTextureSrvHandleCPU(uint32_t index) const {
	return textureSrvHandleCPU_[index];
}

const D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetTextureSrvHandleGPU(uint32_t index) const {
	return textureSrvHandleGPU_[index];
}

const std::vector<D3D12_CPU_DESCRIPTOR_HANDLE>& TextureManager::GetTextureSrvHandleCPUList() const {
	return textureSrvHandleCPU_;
}

const std::vector<D3D12_GPU_DESCRIPTOR_HANDLE>& TextureManager::GetTextureSrvHandleGPUList() const {
	return textureSrvHandleGPU_;
}


