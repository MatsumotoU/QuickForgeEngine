#include "TextureManager.h"
#include <cassert>

#include "../MyString.h"
#include "DirectXCommon.h"

TextureManager* TextureManager::GetInstatnce() {
	static TextureManager instance;
	return &instance;
}

void TextureManager::Initialize(ID3D12Device* device) {
	dxCommon_ = DirectXCommon::GetInstatnce();

	// Comの初期化
	HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);
	assert(SUCCEEDED(hr));

	// デバイスを取得
	assert(device);
	device_ = device;

	// 利用するHeapの設定
	heapProperties_ = {};
	heapProperties_.Type = D3D12_HEAP_TYPE_DEFAULT;

	textureSrvHandleCPU_.clear();
	textureSrvHandleGPU_.clear();
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
		IID_PPV_ARGS(&resource));
	assert(SUCCEEDED(hr));
	return resource;
}

void TextureManager::TransitionResourceBarrier(Microsoft::WRL::ComPtr<ID3D12Resource> texture, ID3D12GraphicsCommandList* commandList) {
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = texture.Get();
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_GENERIC_READ;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
	commandList->ResourceBarrier(1, &barrier);
}

Microsoft::WRL::ComPtr<ID3D12Resource> TextureManager::UploadTextureData(Microsoft::WRL::ComPtr<ID3D12Resource> texture, const DirectX::ScratchImage& mipImages, ID3D12GraphicsCommandList* commandList) {
	std::vector<D3D12_SUBRESOURCE_DATA> subresources;
	subresources.clear();
	DirectX::PrepareUpload(device_, mipImages.GetImages(), mipImages.GetImageCount(), mipImages.GetMetadata(), subresources);
	uint64_t intermediatSize = GetRequiredIntermediateSize(texture.Get(), 0, static_cast<UINT>(subresources.size()));
	Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource = CreateBufferResource(device_, intermediatSize);
	UpdateSubresources(commandList, texture.Get(), intermediateResource.Get(), 0, 0, static_cast<UINT>(subresources.size()), subresources.data());

	EndUploadTextureData(texture.Get(), commandList);
	return intermediateResource;
}

void TextureManager::EndUploadTextureData(Microsoft::WRL::ComPtr<ID3D12Resource> texture, ID3D12GraphicsCommandList* commandList) {
	// Textureへの転送後は利用できるようにD3D12_RESOURCE_STATE_COPY_DESTからD3D12_RESOURCE_STATE_GENERIC_READへResouceStateを変更する
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = texture.Get();
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

void TextureManager::CreateShaderResourceView(const DirectX::TexMetadata& metadata,ID3D12DescriptorHeap* srvDescriptorHeap, Microsoft::WRL::ComPtr<ID3D12Resource> textureResource,uint32_t index) {
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
	device_->CreateShaderResourceView(textureResource.Get(), &srvDesc, textureSrvHandleCPU_[textureSrvHandleCPU_.size() - 1]);
}

D3D12_CPU_DESCRIPTOR_HANDLE TextureManager::GetTextureSrvHandleCPU(uint32_t index) {
	return textureSrvHandleCPU_[index];
}

D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetTextureSrvHandleGPU(uint32_t index) {
	return textureSrvHandleGPU_[index];
}
