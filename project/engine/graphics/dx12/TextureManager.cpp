/**
 * @file TextureManager.cpp
 * @brief テクスチャ管理クラスの実装
 */

#include "TextureManager.h"
#include <cassert>

#include "string/MyString.h"
#include "descriptors/SrvDescriptorHeap.h"
#include "buffer/BufferGenerater/BufferGenerator.h"

#include "EngineDefines.h"
#include "file/FileUtility.h"

namespace {
	std::string kDummyBlackCubeMapKey = "DummyBlackCubeMap";
	std::string kDummyDummyWhite1x1TextureMapKey = "DummyWhite1x1Texture";
	const int32_t kMaxTextures = 512;
	const int32_t kMaxIntermediateResources = 128;
}

using namespace QFE::GRAPHIC::INTERNAL;

TextureManager::TextureManager() :
	device_(nullptr),
	commandList_(nullptr),
	srvDescriptorHeap_(nullptr),
	textureHandle_(0),
	textureSrvHandleCPU_(kMaxTextures),
	textureSrvHandleGPU_(kMaxTextures),
	textureResources_(kMaxTextures),
	scratchImages_(kMaxTextures),
	intermediateResource_(kMaxIntermediateResources) {
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
	filePathLibrary_.Init("TextureFileName");

	textureHandle_ = 0;

	dummyBlackCubeMapHandle_ = CreateDummyBlackCubeMap();
	filePathLibrary_.AddStringToLibrary(kDummyBlackCubeMapKey);
	dummyWhite1x1TextureHandle_ = CreateDummyWhite1x1Texture();
	filePathLibrary_.AddStringToLibrary(kDummyDummyWhite1x1TextureMapKey);
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

const int32_t TextureManager::GetDummyBlackCubeMapHandle() const {
	return dummyBlackCubeMapHandle_;
}

const int32_t TextureManager::GetDummyWhite1x1TextureHandle() const {
	return dummyWhite1x1TextureHandle_;
}

DirectX::ScratchImage TextureManager::Load(const std::string& filePath) {
	// テクスチャファイルを読み込んでScratchImageを作成する
	DirectX::ScratchImage image{};
	std::wstring filePathW = ConvertString(filePath);
	HRESULT hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
	assert(SUCCEEDED(hr));

	// ミップマップの生成
	DirectX::ScratchImage mipImages{};
	hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImages);
	assert(SUCCEEDED(hr));

	// 生成したミップマップイメージを返す
	return mipImages;
}

void TextureManager::LoadScratchImage(const std::string& filePath) {
	// ファイルパスにファイルがあるかどうかを確認
	if (!QFE::FILE::HasFile(filePath)) {
		QFE_REPORT_SYSTEM_ERROR(std::string("TextureManager: File not found - ") + filePath, SystemError::Abort);
	}

	// テクスチャファイルを読み込んでScratchImageを作成する
	DirectX::ScratchImage image{};
	std::wstring filePathW = ConvertString(filePath);
	// ファイル拡張子がDDSかどうかを確認して適切なローダーを使用する
	HRESULT hr;
	if (filePathW.ends_with(L".dds") || filePathW.ends_with(L".DDS")) {
		QFE_LOG(std::format("TextureManager: Loading DDS file '{}'", filePath));
		hr = DirectX::LoadFromDDSFile(filePathW.c_str(), DirectX::DDS_FLAGS_NONE, nullptr, image);
	} else {
		QFE_LOG(std::format("TextureManager: Loading WIC file '{}'", filePath));
		hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
	}
	assert(SUCCEEDED(hr));

#ifdef QFE_OPTIMIZE_OFF
	const DirectX::TexMetadata& metadata = image.GetMetadata();
	QFE_LOG(std::format("TextureManager: Loaded texture from '{}'", filePath));
	QFE_LOG(ConvertString(std::format(L"TextureManager: whidth={},height={},arraySize={}", metadata.width, metadata.height, metadata.arraySize)));
#endif // QFE_OPTIMIZE_OFF

	// ミップマップの生成
	if (image.GetMetadata().width * image.GetMetadata().height != 1) {
		scratchImages_.push_back(std::make_unique<DirectX::ScratchImage>());
		// 圧縮テクスチャの場合は、ミップマップの生成に時間がかかるため、ログを出力してユーザーに知らせる
		if (DirectX::IsCompressed(image.GetMetadata().format)) {
			QFE_LOG(std::format("TextureManager: Generating mipmaps for compressed texture '{}'", filePath));
			scratchImages_.back() = std::make_unique<DirectX::ScratchImage>(std::move(image));
		} else {
			QFE_LOG(std::format("TextureManager: Generating mipmaps for texture '{}'", filePath));
			hr = DirectX::GenerateMipMaps(
				image.GetImages(), image.GetImageCount(), image.GetMetadata(),
				DirectX::TEX_FILTER_SRGB, 0, *(scratchImages_.back().get()));
		}

		assert(SUCCEEDED(hr));
		// ミップマップの生成に失敗した場合は例外をスロー
		if (!SUCCEEDED(hr)) {
			QFE_REPORT_SYSTEM_ERROR(std::string("TextureManager: Failed to generate mipmaps for texture - ") + filePath, SystemError::Abort);
		}

	} else {
		// 1x1のテクスチャはミップマップを生成せず、そのまま保存する
		scratchImages_.push_back(std::make_unique<DirectX::ScratchImage>());
		hr = scratchImages_.back()->InitializeFromImage(*image.GetImage(0, 0, 0));
		assert(SUCCEEDED(hr));
		// 1x1テクスチャの初期化に失敗した場合は例外をスロー
		if (!SUCCEEDED(hr)) {
			QFE_REPORT_SYSTEM_ERROR(std::string("TextureManager: Failed to initialize 1x1 texture - ") + filePath, SystemError::Abort);
		}
	}
}

Microsoft::WRL::ComPtr<ID3D12Resource> TextureManager::CreateTextureResource(const DirectX::TexMetadata& metadata) {
	// メタデータの内容をもとにリソース記述子を設定
	resourceDesc_ = {};
	resourceDesc_.Width = static_cast<UINT>(metadata.width); // 横幅
	resourceDesc_.Height = static_cast<UINT>(metadata.height); // 縦幅
	resourceDesc_.MipLevels = static_cast<UINT16>(metadata.mipLevels); // ミップマップのレベル数
	resourceDesc_.DepthOrArraySize = static_cast<UINT16>(metadata.arraySize); // 配列サイズまたは深度
	resourceDesc_.Format = metadata.format;// DXGI_FORMATで指定されたフォーマット
	resourceDesc_.SampleDesc.Count = 1;// サンプル数（マルチサンプリングを使用しない場合は1）
	resourceDesc_.Dimension = D3D12_RESOURCE_DIMENSION(metadata.dimension);// リソースの次元（1D、2D、3Dなど）

	// メタデータの整合性を確認
	if (resourceDesc_.Width == 0 || resourceDesc_.Height == 0 || resourceDesc_.MipLevels == 0 || resourceDesc_.DepthOrArraySize == 0) {
		QFE_REPORT_SYSTEM_ERROR(std::string("TextureManager: Invalid texture metadata - width, height, mipLevels, and arraySize must be greater than 0"), SystemError::Abort);
	}

#ifdef QFE_OPTIMIZE_PFF
#endif // QFE_OPTIMIZE_PFF

	// リソースの作成
	Microsoft::WRL::ComPtr<ID3D12Resource> resource = nullptr;
	HRESULT hr = device_->CreateCommittedResource(
		&heapProperties_,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc_,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(resource.GetAddressOf()));

	// COM関数の呼び出し結果を確認
	assert(SUCCEEDED(hr));
	if (!SUCCEEDED(hr)) {
		QFE_REPORT_SYSTEM_ERROR(std::string("TextureManager: Failed to create texture resource"), SystemError::Abort);
	}
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
	// Textureへのデータ転送後、シェーダーで読み取れるようにD3D12_RESOURCE_STATE_COPY_DESTからD3D12_RESOURCE_STATE_GENERIC_READへ状態遷移する
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
	// メタデータを元にSRVの記述子を設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;// 2Dテクスチャとして設定
	srvDesc.Texture2D.MipLevels = static_cast<UINT>(metadata.mipLevels);

	// SRVの作成
	DescriptorHandles handles = srvDescriptorHeap_->AssignHeap(textureResource, srvDesc);
	textureSrvHandleCPU_.push_back(handles.cpuHandle_);
	textureSrvHandleGPU_.push_back(handles.gpuHandle_);
}

void TextureManager::CreateSkyBoxShaderResourceView(const DirectX::TexMetadata& metadata, ID3D12Resource* textureResource) {
	// メタデータを元にSRVの記述子を設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;// キューブマップテクスチャとして設定
	srvDesc.TextureCube.MipLevels = static_cast<UINT>(metadata.mipLevels);
	// SRVの作成
	DescriptorHandles handles = srvDescriptorHeap_->AssignHeap(textureResource, srvDesc);
	textureSrvHandleCPU_.push_back(handles.cpuHandle_);
	textureSrvHandleGPU_.push_back(handles.gpuHandle_);
}

int32_t TextureManager::CreateDummyBlackCubeMap() {
	// 空のScratchImageを作成
	auto scratchImage = std::make_unique<DirectX::ScratchImage>();

	// 1x1サイズ、RGBA8ビット、1キューブ(6面)、ミップマップレベル1で初期化
	HRESULT hr = scratchImage->InitializeCube(DXGI_FORMAT_R8G8B8A8_UNORM, 1, 1, 1, 1);
	if (!SUCCEEDED(hr)) {
		QFE_REPORT_SYSTEM_ERROR("TextureManager: Failed to initialize dummy cube map", SystemError::Abort);
	}

	// 全ての面のピクセルデータを0（黒）で塗りつぶす
	const DirectX::Image* images = scratchImage->GetImages();
	for (size_t i = 0; i < scratchImage->GetImageCount(); ++i) {
		memset(images[i].pixels, 0, images[i].rowPitch * images[i].height);
	}

	// 生成したScratchImageを管理リストに追加
	scratchImages_.push_back(std::move(scratchImage));
	const DirectX::TexMetadata& metadata = scratchImages_.back()->GetMetadata();

	// リソース作成
	textureResources_.push_back(CreateTextureResource(metadata));

	// CubeMap用のSRV作成
	CreateSkyBoxShaderResourceView(metadata, textureResources_.back().Get());

	textureHandle_++;

	// データ転送
	intermediateResource_.push_back(
		UploadTextureData(textureResources_.back().Get(), *(scratchImages_.back().get()), commandList_));

	QFE_LOG("TextureManager: Created Dummy Black CubeMap");

	return textureHandle_ - 1;
}

int32_t TextureManager::CreateDummyWhite1x1Texture() {
	// 空のScratchImageを作成
	auto scratchImage = std::make_unique<DirectX::ScratchImage>();
	// 1x1サイズ、RGBA8ビット、ミップマップレベル1で初期化
	HRESULT hr = scratchImage->Initialize2D(DXGI_FORMAT_R8G8B8A8_UNORM, 1, 1, 1, 1);
	if (!SUCCEEDED(hr)) {
		QFE_REPORT_SYSTEM_ERROR("TextureManager: Failed to initialize dummy white texture", SystemError::Abort);
	}
	// ピクセルデータを255（白）で塗りつぶす
	const DirectX::Image* images = scratchImage->GetImages();
	memset(images[0].pixels, 255, images[0].rowPitch * images[0].height);
	// 生成したScratchImageを管理リストに追加
	scratchImages_.push_back(std::move(scratchImage));
	const DirectX::TexMetadata& metadata = scratchImages_.back()->GetMetadata();
	// リソース作成
	textureResources_.push_back(CreateTextureResource(metadata));
	// SRV作成
	CreateShaderResourceView(metadata, textureResources_.back().Get());
	textureHandle_++;
	// データ転送
	intermediateResource_.push_back(
		UploadTextureData(textureResources_.back().Get(), *(scratchImages_.back().get()), commandList_));

	QFE_LOG("TextureManager: Created Dummy White 1x1 Texture");
	return textureHandle_ - 1;
}

void TextureManager::ReleaseIntermediateResources() {
	for (int32_t i = 0; i < intermediateResource_.size(); i++) {
		intermediateResource_[i].Reset();
	}
	intermediateResource_.clear();
}

int32_t TextureManager::LoadTexture(const std::string& filePath) {
	// ファイルパスをログに出力
	QFE_LOG(std::format("TextureManager: LoadPath {}", filePath));

	// 既に読み込み済みのファイルかどうかチェック
	int32_t fileIndex = filePathLibrary_.GetLibraryIndex(filePath);
	if (fileIndex >= 0) {
		QFE_LOG(ConvertString(std::format(L"TextureManager: LoadedTheSameFile->return {}", fileIndex)));
		return fileIndex;
	}

	// 新規ファイル読み込み
	LoadScratchImage(filePath);
	const DirectX::TexMetadata& metadata = scratchImages_.back().get()->GetMetadata();
	textureResources_.push_back(CreateTextureResource(metadata));
#ifdef QFE_OPTIMIZE_OFF
	const auto& resource = textureResources_.back();
	if (resource) {
		D3D12_RESOURCE_DESC desc = resource->GetDesc();
		D3D12_RESOURCE_ALLOCATION_INFO allocInfo = device_->GetResourceAllocationInfo(0, 1, &desc);
		QFE_LOG(std::format("ResourceSize: {}byte", allocInfo.SizeInBytes));
	}
#endif // QFE_OPTIMIZE_OFF
	// ファイル拡張子がDDSかどうかを確認して適切なSRVを作成する
	if (filePath.ends_with(".dds") || filePath.ends_with(".DDS")) {
		CreateSkyBoxShaderResourceView(metadata, textureResources_.back().Get());
	} else {
		CreateShaderResourceView(metadata, textureResources_.back().Get());
	}

	textureHandle_++;
	intermediateResource_.push_back(
		UploadTextureData(textureResources_.back().Get(), *(scratchImages_.back().get()), commandList_));
	QFE_LOG(ConvertString(std::format(L"TextureManager: whidth={},height={},return->{}", metadata.width, metadata.height, textureHandle_ - 1)));
	filePathLibrary_.AddStringToLibrary(filePath);
	return textureHandle_ - 1;
}

QFE::MATH::Vector2 TextureManager::GetTextureSize(int32_t textureHandle) {
	if (textureHandle < 0 || textureHandle >= static_cast<int32_t>(textureResources_.size())) {
		QFE_LOG("TextureManager: Invalid texture handle");
		return QFE::MATH::Vector2(0.0f, 0.0f);
	}
	const DirectX::TexMetadata& metadata = scratchImages_[textureHandle].get()->GetMetadata();
	return QFE::MATH::Vector2(static_cast<float>(metadata.width), static_cast<float>(metadata.height));
}

const D3D12_CPU_DESCRIPTOR_HANDLE TextureManager::GetTextureSrvHandleCPU(uint32_t index) const {
	return textureSrvHandleCPU_[index];
}

const D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetTextureSrvHandleGPU(uint32_t index) const {
	return textureSrvHandleGPU_[index];
}