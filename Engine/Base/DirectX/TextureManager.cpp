#include "TextureManager.h"
#include <cassert>

#include "Utility/MyString.h"
#include "DirectXCommon.h"
#include "ImGuiManager.h"

#include "Descriptors/SrvDescriptorHeap.h"

#ifdef _DEBUG
#include "ImGuiManager.h"
#include "Utility/MyDebugLog.h"
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

	CreateOffscreenShaderResourceView();

#ifdef _DEBUG
	debugTextureIndex_ = 0;
#endif

}

void TextureManager::Finalize() {
	CoUninitialize();
}

// TODO: ロードされていいる画像たちを描画する
#ifdef _DEBUG
void TextureManager::DrawImages() {
}
#endif // _DEBUG

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
	uint64_t intermediatSize = GetRequiredIntermediateSize(texture, 0, static_cast<UINT>(subresources.size()));
	Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource = CreateBufferResource(device_, intermediatSize);
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

void TextureManager::CreateShaderResourceView(const DirectX::TexMetadata& metadata, ID3D12Resource* textureResource, uint32_t index) {
	// metaDataを基にSRVの設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;// 2Dテクスチャ
	srvDesc.Texture2D.MipLevels = static_cast<UINT>(metadata.mipLevels);

	// SRVを作成するDescriptorHeapの場所を決める
	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU{};
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU{};
	textureSrvHandleCPU_.push_back(textureSrvHandleCPU);
	textureSrvHandleGPU_.push_back(textureSrvHandleGPU);
	DescriptorHandles handles = srvDescriptorHeap_->AssignTextureHandles(index);
	textureSrvHandleCPU_[textureSrvHandleCPU_.size() - 1] = handles.cpuHandle_;
	textureSrvHandleGPU_[textureSrvHandleCPU_.size() - 1] = handles.gpuHandle_;
	// SRVの作成
	srvDescriptorHeap_->AssignHeap(textureResource, srvDesc, handles.cpuHandle_);
}

void TextureManager::CreateOffscreenShaderResourceView() {
	// オフスクリーン用のSRV登録
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = dxCommon_->GetOffscreenResource(0)->GetDesc().Format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	// SRVを作成するディスクリプタヒープの場所を決める
	DescriptorHandles handles = srvDescriptorHeap_->AssignOffscreenHandles(1);
	offscreenSrvHandleCPU_[0] = handles.cpuHandle_;
	offscreenSrvHandleGPU_[0] = handles.gpuHandle_;
	handles = srvDescriptorHeap_->AssignOffscreenHandles(2);
	offscreenSrvHandleCPU_[1] = handles.cpuHandle_;
	offscreenSrvHandleGPU_[1] = handles.gpuHandle_;

	// SRVの作成
	srvDescriptorHeap_->AssignHeap(dxCommon_->GetOffscreenResource(0), srvDesc, offscreenSrvHandleCPU_[0]);
	srvDescriptorHeap_->AssignHeap(dxCommon_->GetOffscreenResource(1), srvDesc, offscreenSrvHandleCPU_[1]);
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
	CreateShaderResourceView(metadata, textureResource.Get(), 3 + textureHandle_);
	textureHandle_++;
	textureResources_.push_back(textureResource);
	intermediateResource_.push_back(
		UploadTextureData(textureResources_[textureResources_.size() - 1].Get(), scratchImages_[scratchImages_.size() - 1], dxCommon_->GetCommandList()));
#ifdef _DEBUG
	DebugLog(ConvertString(std::format(L"TextureManager: whidth={},height={},return->{}", metadata.width,metadata.height,textureHandle_-1)));
#endif // _DEBUG
	filePathLiblary_.AddStringToLiblary(filePath);
	return textureHandle_ - 1;
}

Vector2 TextureManager::GetTextureSize(int32_t textureHandle) {
	if (textureHandle < 0 || textureHandle >= static_cast<int32_t>(textureResources_.size())) {
		DebugLog("TextureManager: Invalid texture handle");
		return Vector2(0.0f, 0.0f);
	}
	const DirectX::TexMetadata& metadata = scratchImages_[textureHandle].GetMetadata();
	return Vector2(static_cast<float>(metadata.width), static_cast<float>(metadata.height));
}

#ifdef _DEBUG
void TextureManager::DrawImGui() {
	ImGui::SliderInt("Texture Handle", &debugTextureIndex_, 0, static_cast<int>(textureSrvHandleGPU_.size()) - 1);
	ImGui::SameLine();
	ImGui::Text("Texture Count: %d", textureHandle_);
	if (textureResources_.size() > 0) {
		ImGui::Image(textureSrvHandleGPU_[debugTextureIndex_].ptr, ImVec2(256, 256), ImVec2(0, 0), ImVec2(1, 1));
	} else {
		ImGui::Text("No Texture Loaded");
	}
}
#endif // _DEBUG

D3D12_CPU_DESCRIPTOR_HANDLE TextureManager::GetTextureSrvHandleCPU(uint32_t index) {
	return textureSrvHandleCPU_[index];
}

D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetTextureSrvHandleGPU(uint32_t index) {
	return textureSrvHandleGPU_[index];
}

D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetOffscreenSrvHandleGPU(uint32_t index) {
	return offscreenSrvHandleGPU_[index];
}
