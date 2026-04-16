/**
 * @file TextureManager.cpp
 * @brief 繝・け繧ｹ繝√Ε邂｡逅・け繝ｩ繧ｹ縺ｮ螳溯｣・
 */

#include "engine/include/assets/2DTexture/TextureManager.h"
#include <cassert>

#include "engine/include/utility/String/MyString.h"
#include "engine/include/Graphic/DirectXCommon/DirectXCommon.h"
#include "engine/include/Graphic/DirectXCommon/Descriptors/SrvDescriptorHeap.h"
#include "engine/include/Graphic/ShaderBuffer/BufferGenerater/BufferGenerator.h"

#ifdef QFE_OPTIMIZE_OFF
#include "engine/include/utility/DebugTool/DebugLog/MyDebugLog.h"
#include "engine/include/utility/DebugTool/ImGui/ImGuiInclude.h"
#endif // QFE_OPTIMIZE_OFF

using namespace QFE;

TextureManager::TextureManager() :
	device_(nullptr),
	commandList_(nullptr),
	srvDescriptorHeap_(nullptr),
	textureHandle_(0) {
}

/// @brief 初期化処理
void TextureManager::Initialize(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, SrvDescriptorHeap* srvDescriptorHeap) {
	srvDescriptorHeap_ = srvDescriptorHeap;

	// COMの初期化
	HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);
	assert(SUCCEEDED(hr));
	hr;

	// デバイスとコマンドリストの確認
	assert(device);
	device_ = device;
	assert(commandList);
	commandList_ = commandList;

	// ヒーププロパティの設定
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

/// @brief 終了処理
void TextureManager::Finalize() {
	// リソースの解放
	textureSrvHandleCPU_.clear();
	textureSrvHandleGPU_.clear();
	textureResources_.clear();
	scratchImages_.clear();
	intermediateResource_.clear();

	CoUninitialize();
}

DirectX::ScratchImage TextureManager::Load(const std::string& filePath) {
	// 郢昴・縺醍ｹｧ・ｹ郢昶・ﾎ慕ｹ晁ｼ斐＜郢ｧ・､郢晢ｽｫ郢ｧ螳夲ｽｪ・ｭ邵ｺ・ｿ髴趣ｽｼ郢ｧ阮吶€堤ｹ晏干ﾎ溽ｹｧ・ｰ郢晢ｽｩ郢晢｣ｰ邵ｺ・ｧ闖ｴ・ｿ邵ｺ蛹ｻ・狗ｹｧ蛹ｻ竕ｧ邵ｺ・ｫ邵ｺ蜷ｶ・・
	DirectX::ScratchImage image{};
	std::wstring filePathW = ConvertString(filePath);
	HRESULT hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
	assert(SUCCEEDED(hr));

	// 郢晄ｺ倥Ε郢晏干繝ｻ郢昴・繝ｻ邵ｺ・ｮ闖ｴ諛医・
	DirectX::ScratchImage mipImages{};
	hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImages);
	assert(SUCCEEDED(hr));

	// 郢晄ｺ倥Ε郢晏ｶｺ・ｻ蛟･窶ｳ邵ｺ・ｮ郢昴・繝ｻ郢ｧ・ｿ郢ｧ螳夲ｽｿ譁絶・
	return mipImages;
}

void TextureManager::LoadScratchImage(const std::string& filePath) {
	// 郢昴・縺醍ｹｧ・ｹ郢昶・ﾎ慕ｹ晁ｼ斐＜郢ｧ・､郢晢ｽｫ郢ｧ螳夲ｽｪ・ｭ邵ｺ・ｿ髴趣ｽｼ郢ｧ阮吶€堤ｹ晏干ﾎ溽ｹｧ・ｰ郢晢ｽｩ郢晢｣ｰ邵ｺ・ｧ闖ｴ・ｿ邵ｺ蛹ｻ・狗ｹｧ蛹ｻ竕ｧ邵ｺ・ｫ邵ｺ蜷ｶ・・
	DirectX::ScratchImage image{};
	std::wstring filePathW = ConvertString(filePath);
	HRESULT hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
	assert(SUCCEEDED(hr));

#ifdef QFE_OPTIMIZE_OFF
	const DirectX::TexMetadata& metadata = image.GetMetadata();
	DebugLog(ConvertString(std::format(L"TextureManager: whidth={},height={},arraySize={}", metadata.width, metadata.height, metadata.arraySize)));
#endif // QFE_OPTIMIZE_OFF

	// 郢晄ｺ倥Ε郢晏干繝ｻ郢昴・繝ｻ邵ｺ・ｮ闖ｴ諛医・
	if (image.GetMetadata().width * image.GetMetadata().height != 1) {
		scratchImages_.push_back(std::make_unique<DirectX::ScratchImage>());
		hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, *(scratchImages_.back().get()));
		assert(SUCCEEDED(hr));
	} else {
		// 邵ｺ譏ｴ繝ｻ邵ｺ・ｾ邵ｺ・ｾ隴ｬ・ｼ驍上・
		scratchImages_.push_back(std::make_unique<DirectX::ScratchImage>(std::move(image)));
	}
}

Microsoft::WRL::ComPtr<ID3D12Resource> TextureManager::CreateTextureResource(const DirectX::TexMetadata& metadata) {
	// metadata郢ｧ雋樊ｸ慕ｸｺ・ｫResource邵ｺ・ｮ髫ｪ・ｭ陞ｳ繝ｻ
	resourceDesc_ = {};
	resourceDesc_.Width = static_cast<UINT>(metadata.width); // 郢昴・縺醍ｹｧ・ｹ郢昶・ﾎ慕ｸｺ・ｮ陝ｷ繝ｻ
	resourceDesc_.Height = static_cast<UINT>(metadata.height); // 郢昴・縺醍ｹｧ・ｹ郢昶・ﾎ慕ｸｺ・ｮ鬯ｮ蛟･・・
	resourceDesc_.MipLevels = static_cast<UINT16>(metadata.mipLevels); // mipmap邵ｺ・ｮ隰ｨ・ｰ
	resourceDesc_.DepthOrArraySize = static_cast<UINT16>(metadata.arraySize); // 陞ゑｽ･髯ｦ邨罫鬩滓ｦ翫・郢昴・縺醍ｹｧ・ｹ郢昶・ﾎ慕ｸｺ・ｮ鬩滓ｦ翫・隰ｨ・ｰ
	resourceDesc_.Format = metadata.format;
	resourceDesc_.SampleDesc.Count = 1;
	resourceDesc_.Dimension = D3D12_RESOURCE_DIMENSION(metadata.dimension);

	// 郢晢ｽｪ郢ｧ・ｽ郢晢ｽｼ郢ｧ・ｹ邵ｺ・ｮ騾墓ｻ薙・
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
	// Texture邵ｺ・ｸ邵ｺ・ｮ髴・ｽ｢鬨ｾ竏晢ｽｾ蠕後・陋ｻ・ｩ騾包ｽｨ邵ｺ・ｧ邵ｺ髦ｪ・狗ｹｧ蛹ｻ竕ｧ邵ｺ・ｫD3D12_RESOURCE_STATE_COPY_DEST邵ｺ荵晢ｽ吋3D12_RESOURCE_STATE_GENERIC_READ邵ｺ・ｸResouceState郢ｧ雋橸ｽ､逕ｻ蟲ｩ邵ｺ蜷ｶ・・
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
	// metaData郢ｧ雋樊ｸ慕ｸｺ・ｫSRV邵ｺ・ｮ髫ｪ・ｭ陞ｳ繝ｻ
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;// 2D郢昴・縺醍ｹｧ・ｹ郢昶・ﾎ・
	srvDesc.Texture2D.MipLevels = static_cast<UINT>(metadata.mipLevels);

	// SRV邵ｺ・ｮ闖ｴ諛医・
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
	// 郢晁ｼ斐＜郢ｧ・､郢晢ｽｫ郢昜ｻ｣縺幃勗・ｨ驕会ｽｺ
#ifdef QFE_OPTIMIZE_OFF
	DebugLog(std::format("TextureManager: LoadPath {}", filePath));
#endif // QFE_OPTIMIZE_OFF

	// 陷ｷ蠕個ｧ騾包ｽｻ陷剃ｸ翫Ψ郢ｧ・｡郢ｧ・､郢晢ｽｫ郢ｧ螳夲ｽｪ・ｭ邵ｺ・ｿ髴趣ｽｼ邵ｺ・ｾ邵ｺ・ｪ邵ｺ繝ｻ
	int32_t fileIndex = filePathLiblary_.GetLibraryIndex(filePath);
	if (fileIndex >= 0) {
#ifdef QFE_OPTIMIZE_OFF
		DebugLog(ConvertString(std::format(L"TextureManager: LoadedTheSameFile->return {}", fileIndex)));
#endif // QFE_OPTIMIZE_OFF
		return fileIndex;
	}

	// 騾包ｽｻ陷貞臆・ｪ・ｭ邵ｺ・ｿ髴趣ｽｼ邵ｺ・ｿ陷・ｽｦ騾・・
	LoadScratchImage(filePath);
	const DirectX::TexMetadata& metadata = scratchImages_.back().get()->GetMetadata();
	textureResources_.push_back(CreateTextureResource(metadata));
#ifdef QFE_OPTIMIZE_OFF
	const auto& resource = textureResources_.back();
	if (resource) {
		D3D12_RESOURCE_DESC desc = resource->GetDesc();
		D3D12_RESOURCE_ALLOCATION_INFO allocInfo = device_->GetResourceAllocationInfo(0, 1, &desc);
		DebugLog(std::format("ResourceSize: {}byte", allocInfo.SizeInBytes));
	}
#endif // QFE_OPTIMIZE_OFF
	CreateShaderResourceView(metadata, textureResources_.back().Get());
	textureHandle_++;
	intermediateResource_.push_back(
		UploadTextureData(textureResources_.back().Get(), *(scratchImages_.back().get()), commandList_));
#ifdef QFE_OPTIMIZE_OFF
	DebugLog(ConvertString(std::format(L"TextureManager: whidth={},height={},return->{}", metadata.width, metadata.height, textureHandle_ - 1)));
#endif // QFE_OPTIMIZE_OFF
	filePathLiblary_.AddStringToLibrary(filePath);
	return textureHandle_ - 1;
}

Vector2 TextureManager::GetTextureSize(int32_t textureHandle) {
	if (textureHandle < 0 || textureHandle >= static_cast<int32_t>(textureResources_.size())) {
#ifdef QFE_OPTIMIZE_OFF
		DebugLog("TextureManager: Invalid texture handle");
#endif // QFE_OPTIMIZE_OFF
		return Vector2(0.0f, 0.0f);
	}
	const DirectX::TexMetadata& metadata = scratchImages_[textureHandle].get()->GetMetadata();
	return Vector2(static_cast<float>(metadata.width), static_cast<float>(metadata.height));
}

const D3D12_CPU_DESCRIPTOR_HANDLE TextureManager::GetTextureSrvHandleCPU(uint32_t index) const {
	return textureSrvHandleCPU_[index];
}

const D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetTextureSrvHandleGPU(uint32_t index) const {
	return textureSrvHandleGPU_[index];
}