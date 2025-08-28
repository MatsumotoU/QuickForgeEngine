#include "TextureManager.h"
#include <cassert>

#include "Utility/String/MyString.h"
#include "Platform/Windows/Graphic/DirectXCommon/DirectXCommon.h"
#include "Platform/Windows/Graphic/DirectXCommon/Descriptors/SrvDescriptorHeap.h"
#include "Platform/Windows/Graphic/ShaderBuffer/BufferGenerater/BufferGenerator.h"

#ifdef _DEBUG
#include "AppUtility/DebugTool/DebugLog/MyDebugLog.h"
#endif // _DEBUG

void TextureManager::Initialize(DirectXCommon* dxCommon, SrvDescriptorHeap* srvDescriptorHeap) {
	dxCommon_ = dxCommon;
	srvDescriptorHeap_ = srvDescriptorHeap;

	// Comの初期化
	HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);
	assert(SUCCEEDED(hr));
	hr;

	// デバイスを取得
	assert(dxCommon_);
	device_ = dxCommon_->GetDevice();

	// 利用するHeapの設定
	heapProperties_ = {};
	heapProperties_.Type = D3D12_HEAP_TYPE_DEFAULT;

	textureSrvHandleCPU_.clear();
	textureSrvHandleGPU_.clear();
	textureResources_.clear();
	scratchImages_.clear();
	intermediateResource_.clear();
	filePathLiblary_.Init("TextureFileName");

	textureHandle_ = 0;

#ifdef _DEBUG
	debugTextureIndex_ = 0;
#endif

}

void TextureManager::Finalize() {
	CoUninitialize();
}

DirectX::ScratchImage TextureManager::Load(const std::string& filePath) {
	// テクスチャファイルを読み込んでプログラムで使えるようにする
	DirectX::ScratchImage image{};
	std::wstring filePathW = ConvertString(filePath);
	HRESULT hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
	assert(SUCCEEDED(hr));

	// ミップマップの作成
	DirectX::ScratchImage mipImages{};
	hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImages);
	assert(SUCCEEDED(hr));

	// ミップ付きのデータを返す
	return mipImages;
}

void TextureManager::LoadScratchImage(const std::string& filePath) {
	scratchImages_.push_back(DirectX::ScratchImage());
	// テクスチャファイルを読み込んでプログラムで使えるようにする
	DirectX::ScratchImage image{};
	std::wstring filePathW = ConvertString(filePath);
	HRESULT hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
	assert(SUCCEEDED(hr));

#ifdef _DEBUG
	const DirectX::TexMetadata& metadata = image.GetMetadata();
	DebugLog(ConvertString(std::format(L"TextureManager: whidth={},height={},arraySize={}", metadata.width, metadata.height,metadata.arraySize)));
#endif // _DEBUG

	// ミップマップの作成
	if (image.GetMetadata().width * image.GetMetadata().height != 1) {
		hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, scratchImages_.back());
		assert(SUCCEEDED(hr));
	} else {
		// そのまま格納
		scratchImages_.push_back(std::move(image));
	}
}

Microsoft::WRL::ComPtr<ID3D12Resource> TextureManager::CreateTextureResource(const DirectX::TexMetadata& metadata) {
	// metadataを基にResourceの設定
	resourceDesc_ = {};
	resourceDesc_.Width = static_cast<UINT>(metadata.width); // テクスチャの幅
	resourceDesc_.Height = static_cast<UINT>(metadata.height); // テクスチャの高さ
	resourceDesc_.MipLevels = static_cast<UINT16>(metadata.mipLevels); // mipmapの数
	resourceDesc_.DepthOrArraySize = static_cast<UINT16>(metadata.arraySize); // 奥行or配列テクスチャの配列数
	resourceDesc_.Format = metadata.format;
	resourceDesc_.SampleDesc.Count = 1;
	resourceDesc_.Dimension = D3D12_RESOURCE_DIMENSION(metadata.dimension);

	// リソースの生成
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
	
	Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource = BufferGenerator::Generate(device_, subresources.size());
	UpdateSubresources(commandList, texture, intermediateResource.Get(), 0, 0, static_cast<UINT>(subresources.size()), subresources.data());

	EndUploadTextureData(texture, commandList);
	return intermediateResource;
}

void TextureManager::EndUploadTextureData(ID3D12Resource* texture, ID3D12GraphicsCommandList* commandList) {
	// Textureへの転送後は利用できるようにD3D12_RESOURCE_STATE_COPY_DESTからD3D12_RESOURCE_STATE_GENERIC_READへResouceStateを変更する
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
	// metaDataを基にSRVの設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;// 2Dテクスチャ
	srvDesc.Texture2D.MipLevels = static_cast<UINT>(metadata.mipLevels);

	// SRVの作成
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
	// ファイルパス表示
#ifdef _DEBUG
	DebugLog(std::format("TextureManager: LoadPath {}", filePath));
#endif // _DEBUG

	// 同じ画像ファイルを読み込まない
	int32_t fileIndex = filePathLiblary_.GetLiblaryIndex(filePath);
	if (fileIndex >= 0) {
#ifdef _DEBUG
		DebugLog(ConvertString(std::format(L"TextureManager: LoadedTheSameFile->return {}", fileIndex)));
#endif // _DEBUG
		return fileIndex;
	}

	// 画像読み込み処理
	LoadScratchImage(filePath);
	const DirectX::TexMetadata& metadata = scratchImages_.back().GetMetadata();
	Microsoft::WRL::ComPtr<ID3D12Resource> textureResource = CreateTextureResource(metadata);
	CreateShaderResourceView(metadata, textureResource.Get());
	textureHandle_++;
	textureResources_.push_back(textureResource);
	intermediateResource_.push_back(
		UploadTextureData(textureResources_[textureResources_.size() - 1].Get(), scratchImages_[scratchImages_.size() - 1], dxCommon_->GetCommandManager(D3D12_COMMAND_LIST_TYPE_DIRECT)));
#ifdef _DEBUG
	DebugLog(ConvertString(std::format(L"TextureManager: whidth={},height={},return->{}", metadata.width,metadata.height,textureHandle_-1)));
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

D3D12_CPU_DESCRIPTOR_HANDLE TextureManager::GetTextureSrvHandleCPU(uint32_t index) {
	return textureSrvHandleCPU_[index];
}

D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetTextureSrvHandleGPU(uint32_t index) {
	return textureSrvHandleGPU_[index];
}