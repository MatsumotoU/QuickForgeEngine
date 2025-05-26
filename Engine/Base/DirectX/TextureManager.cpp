#include "TextureManager.h"
#include <cassert>

#include "../MyString.h"
#include "DirectXCommon.h"
#include "ImGuiManager.h"

#include "Descriptors/SrvDescriptorHeap.h"

void TextureManager::Initialize(DirectXCommon* dxCommon, SrvDescriptorHeap* srvDescriptorHeap) {
	dxCommon_ = dxCommon;
	srvDescriptorHeap_ = srvDescriptorHeap;

	// Comの初期化
	HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);
	assert(SUCCEEDED(hr));

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
	filePathliblary_.clear();

	textureHandle_ = 0;

	CreateOffscreenShaderResourceView();
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

	// ミップマップの作成
	hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, scratchImages_.back());
	assert(SUCCEEDED(hr));
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

//void TextureManager::UploadTextureData(ID3D12Resource* texture, const DirectX::ScratchImage& mipImages) {
//	// Meta情報の取得
//	const DirectX::TexMetadata& metaData = mipImages.GetMetadata();
//	// 全MipMapについて
//	for (size_t mipLevel = 0; mipLevel < metaData.mipLevels; ++mipLevel) {
//		// MipMapLevelを指定して各Imageを取得
//		const DirectX::Image* img = mipImages.GetImage(mipLevel, 0, 0);
//		// Textureに転送
//		HRESULT hr = texture->WriteToSubresource(
//			static_cast<UINT>(mipLevel),
//			nullptr,
//			img->pixels,
//			static_cast<UINT>(img->rowPitch),
//			static_cast<UINT>(img->slicePitch));
//		assert(SUCCEEDED(hr));
//	}
//}

void TextureManager::CreateShaderResourceView(const DirectX::TexMetadata& metadata,ID3D12DescriptorHeap* srvDescriptorHeap,ID3D12Resource* textureResource,uint32_t index) {
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
	textureSrvHandleCPU_[textureSrvHandleCPU_.size() - 1] = GetCPUDecriptorHandle(srvDescriptorHeap, dxCommon_->GetDescriptorSizeSRV(), index);
	textureSrvHandleGPU_[textureSrvHandleCPU_.size() - 1] = GetGPUDecriptorHandle(srvDescriptorHeap, dxCommon_->GetDescriptorSizeSRV(), index);
	// ImGuiの次のDescriptorを使う
	UINT discriptorHeapSize = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	textureSrvHandleCPU_[textureSrvHandleCPU_.size() - 1].ptr += discriptorHeapSize;
	textureSrvHandleGPU_[textureSrvHandleCPU_.size() - 1].ptr += discriptorHeapSize;
	// SRVの作成
	device_->CreateShaderResourceView(textureResource, &srvDesc, textureSrvHandleCPU_[textureSrvHandleCPU_.size() - 1]);
}

void TextureManager::CreateOffscreenShaderResourceView() {
	// オフスクリーン用のSRV登録
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = dxCommon_->GetOffscreenResource()->GetDesc().Format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	// SRVを作成するディスクリプタヒープの場所を決める
	offscreenSrvHandleCPU_ = GetCPUDecriptorHandle(
		srvDescriptorHeap_->GetSrvDescriptorHeap(),
		dxCommon_->GetDescriptorSizeSRV(),
		1
	);
	offscreenSrvHandleGPU_ = GetGPUDecriptorHandle(
		srvDescriptorHeap_->GetSrvDescriptorHeap(),
		dxCommon_->GetDescriptorSizeSRV(),
		1
	);

	// SRVの作成
	device_->CreateShaderResourceView(dxCommon_->GetOffscreenResource(), &srvDesc, offscreenSrvHandleCPU_);
}

void TextureManager::PreDraw() {
	/*for (int32_t i = 0; i < textureResources_.size(); i++) {
		intermediateResource_.push_back(UploadTextureData(textureResources_[i].Get(), scratchImages_[i], dxCommon_->GetCommandList()));
	}*/
}

void TextureManager::PostDraw() {
	/*for (int32_t i = 0; i < textureResources_.size(); i++) {
		TransitionResourceBarrier(textureResources_[i].Get(), dxCommon_->GetCommandList());
	}*/
}

void TextureManager::ReleaseIntermediateResources() {
	for (int32_t i = 0; i < intermediateResource_.size(); i++) {
		intermediateResource_[i].Reset();
	}
	intermediateResource_.clear();
}

int32_t TextureManager::LoadTexture(const std::string& filePath) {
	// 同じ画像ファイルを読み込まない
	for (int i = 0; i < filePathliblary_.size(); i++) {
		if (filePathliblary_[i] == filePath) {
			return i;
		}
	}
	filePathliblary_.push_back(filePath);
	
	// 画像読み込み処理
	LoadScratchImage(filePath);
	const DirectX::TexMetadata& metadata = scratchImages_.back().GetMetadata();
	Microsoft::WRL::ComPtr<ID3D12Resource> textureResource = CreateTextureResource(metadata);
	CreateShaderResourceView(metadata, srvDescriptorHeap_->GetSrvDescriptorHeap(), textureResource.Get(), 2 + textureHandle_);
	textureHandle_++;
	textureResources_.push_back(textureResource);
	intermediateResource_.push_back(
		UploadTextureData(textureResources_[textureResources_.size() - 1].Get(), scratchImages_[textureResources_.size() - 1], dxCommon_->GetCommandList()));
	return textureHandle_-1;
}

D3D12_CPU_DESCRIPTOR_HANDLE TextureManager::GetTextureSrvHandleCPU(uint32_t index) {
	return textureSrvHandleCPU_[index];
}

D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetTextureSrvHandleGPU(uint32_t index) {
	return textureSrvHandleGPU_[index];
}

D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetOffscreenSrvHandleGPU() {
	return offscreenSrvHandleGPU_;
}
