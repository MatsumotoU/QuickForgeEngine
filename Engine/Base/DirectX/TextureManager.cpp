#include "TextureManager.h"
#include <cassert>

#include "../MyString.h"

TextureManager* TextureManager::GetInstatnce() {
	static TextureManager instance;
	return &instance;
}

void TextureManager::Initialize(ID3D12Device* device) {
	// Comの初期化
	HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);
	assert(SUCCEEDED(hr));

	// デバイスを取得
	assert(device);
	device_ = device;

	// 利用するHeapの設定
	heapProperties_ = {};
	heapProperties_.Type = D3D12_HEAP_TYPE_CUSTOM;
	heapProperties_.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
	heapProperties_.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;
}

void TextureManager::Finalize() {
	CoUninitialize();
}

DirectX::ScratchImage TextureManager::LoadTexture(const std::string& filePath) {
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

ID3D12Resource* TextureManager::CreateTextureResource(const DirectX::TexMetadata& metadata) {
	// metadataを基にResourceの設定
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = static_cast<UINT>(metadata.width); // テクスチャの幅
	resourceDesc.Height = static_cast<UINT>(metadata.height); // テクスチャの高さ
	resourceDesc.MipLevels = static_cast<UINT16>(metadata.mipLevels); // mipmapの数
	resourceDesc.DepthOrArraySize = static_cast<UINT16>(metadata.arraySize); // 奥行or配列テクスチャの配列数
	resourceDesc.Format = metadata.format;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION(metadata.dimension);

	// リソースの生成
	ID3D12Resource* resource = nullptr;
	HRESULT hr = device_->CreateCommittedResource(
		&heapProperties_,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&resource));
	assert(SUCCEEDED(hr));
	return resource;
}

void TextureManager::UploadTextureData(ID3D12Resource* texture, const DirectX::ScratchImage& mipImages) {
	// Meta情報の取得
	const DirectX::TexMetadata& metaData = mipImages.GetMetadata();
	// 全MipMapについて
	for (size_t mipLevel = 0; mipLevel < metaData.mipLevels; ++mipLevel) {
		// MipMapLevelを指定して各Imageを取得
		const DirectX::Image* img = mipImages.GetImage(mipLevel, 0, 0);
		// Textureに転送
		HRESULT hr = texture->WriteToSubresource(
			static_cast<UINT>(mipLevel),
			nullptr,
			img->pixels,
			static_cast<UINT>(img->rowPitch),
			static_cast<UINT>(img->slicePitch));
		assert(SUCCEEDED(hr));
	}
}

void TextureManager::CreateShaderResourceView(const DirectX::TexMetadata& metadata,ID3D12DescriptorHeap* srvDescriptorHeap,ID3D12Resource* textureResource) {
	// metaDataを基にSRVの設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;// 2Dテクスチャ
	srvDesc.Texture2D.MipLevels = static_cast<UINT>(metadata.mipLevels);

	// SRVを作成するDescriptorHeapの場所を決める
	textureSrvHandleCPU_ = srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	textureSrvHandleGPU_ = srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
	// ImGuiの次のDescriptorを使う
	UINT discriptorHeapSize = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	textureSrvHandleCPU_.ptr += discriptorHeapSize;
	textureSrvHandleGPU_.ptr += discriptorHeapSize;
	// SRVの作成
	device_->CreateShaderResourceView(textureResource, &srvDesc, textureSrvHandleCPU_);
}

D3D12_CPU_DESCRIPTOR_HANDLE TextureManager::GetTextureSrvHandleCPU() {
	return textureSrvHandleCPU_;
}

D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetTextureSrvHandleGPU() {
	return textureSrvHandleGPU_;
}
