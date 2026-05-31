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
#include "D3D12GraphicEngine.h"

namespace {
	std::string kDummyBlackCubeMapKey = "DummyBlackCubeMap";
	std::string kDummyDummyWhite1x1TextureMapKey = "DummyWhite1x1Texture";
	const int32_t kMaxTextures = 512;
	const int32_t kMaxIntermediateResources = 128;
	const uint32_t kInvalidTextureHandle = UINT32_MAX;
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
	intermediateResource_(kMaxIntermediateResources),
	dummyBlackCubeMapHandle_(kInvalidTextureHandle),
	dummyWhite1x1TextureHandle_(kInvalidTextureHandle) {
}

const uint32_t TextureManager::GetDummyBlackCubeMapHandle() const {
	return dummyBlackCubeMapHandle_;
}

const uint32_t TextureManager::GetDummyWhite1x1TextureHandle() const {
	return dummyWhite1x1TextureHandle_;
}

uint32_t TextureManager::CreateDummyBlackCubeMap() {
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

uint32_t TextureManager::CreateDummyWhite1x1Texture() {
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

uint32_t TextureManager::LoadTexture(const std::string& filePath) {
	// ファイルパスをログに出力
	QFE_LOG(std::format("TextureManager: LoadPath {}", filePath));

	// 既に読み込み済みのファイルかどうかチェック
	int32_t fileIndex = filePathLibrary_.GetLibraryIndex(filePath);
	if (fileIndex >= 0) {
		QFE_LOG(ConvertString(std::format(L"TextureManager: LoadedTheSameFile->return {}", fileIndex)));
		return static_cast<uint32_t>(fileIndex);
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

QFE::MATH::Vector2 TextureManager::GetTextureSize(uint32_t textureHandle) {
	if (textureHandle == kInvalidTextureHandle || textureHandle >= static_cast<uint32_t>(textureResources_.size())) {
		QFE_LOG("TextureManager: Invalid texture handle");
		return QFE::MATH::Vector2(0.0f, 0.0f);
	}
	const DirectX::TexMetadata& metadata = scratchImages_[static_cast<size_t>(textureHandle)].get()->GetMetadata();
	return QFE::MATH::Vector2(static_cast<float>(metadata.width), static_cast<float>(metadata.height));
}