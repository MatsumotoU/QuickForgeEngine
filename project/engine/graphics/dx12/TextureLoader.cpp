#include "TextureLoader.h"
#include <cassert>

#include "string/MyString.h"

#include "EngineDefines.h"
#include "file/FileUtility.h"

using namespace QFE::GRAPHIC;

namespace {
	const std::string kDummyBlackCubeMapKey = "DummyBlackCubeMap";
	const std::string kDummyDummyWhite1x1TextureMapKey = "DummyWhite1x1Texture";
}

void TextureLoader::Initialize(TextureLoaderInitializeInfo info) {
	initializeInfo_ = std::move(info);

	// COMの初期化
	HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);
	assert(SUCCEEDED(hr));
	hr;

	// ダミーの黒いキューブマップを作成して保存する
	CreateTextureFromScratchImage(std::move(CreateDummyBlackCubeMap()), kDummyBlackCubeMapKey, D3D12_SRV_DIMENSION_TEXTURECUBE);
	// ダミーの白い1x1テクスチャを作成して保存する
	CreateTextureFromScratchImage(std::move(CreateDummyWhite1x1Texture()), kDummyDummyWhite1x1TextureMapKey, D3D12_SRV_DIMENSION_TEXTURE2D);
}
/// @brief 終了処理
void TextureLoader::Finalize() {
#ifdef QFE_OPTIMIZE_OFF
	QFE_LOG("TextureLoader: Finalize, releasing all textures");
	for (const auto& [name, data] : textureDataMap_) {
		QFE_LOG(std::format("TextureLoader: Releasing texture '{}', handle {}", name, static_cast<uint32_t>(data.resourceHandle)));
	}
#endif

	textureDataMap_.clear();	
	CoUninitialize();
}

DirectXResourceHandle TextureLoader::LoadTexture(const std::string& filePath) {
	QFE_LOG(std::format("TextureLoader: LoadPath {}", filePath));
	std::string fileName = QFE::FILE::GetFileName(filePath);

	// 既に読み込み済みのファイルかどうかチェック（キーは一貫して fileName を使う）
	if (textureDataMap_.contains(fileName)) {
		DirectXResourceHandle existingHandle = textureDataMap_[fileName].resourceHandle;
		QFE_LOG(
			std::format("TextureLoader: Texture '{}' already loaded, returning existing handle {}",
			filePath, static_cast<uint32_t>(existingHandle)));
		return existingHandle;
	}

	// ファイルからScratchImageをロードする
	DirectX::ScratchImage scratchImage = LoadScratchImageFromFile(filePath);

	// DDSかそれ以外かでSRVの形を判定
	D3D12_SRV_DIMENSION dimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	if (filePath.ends_with(".dds") || filePath.ends_with(".DDS")) {
		dimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
	}

	// 内部でリソース作成、SRV作成、アップロード、マップ保存
	return CreateTextureFromScratchImage(std::move(scratchImage), fileName, dimension);
}

const DirectXResourceHandle TextureLoader::GetDummyBlackCubeMapHandle() const {
	return textureDataMap_.at(kDummyBlackCubeMapKey).resourceHandle;
}

const DirectXResourceHandle TextureLoader::GetDummyWhite1x1TextureHandle() const {
	return textureDataMap_.at(kDummyDummyWhite1x1TextureMapKey).resourceHandle;
}

DirectX::ScratchImage QFE::GRAPHIC::TextureLoader::LoadScratchImageFromFile(const std::string& filePath) {
	// ファイルパスにファイルがあるかどうかを確認
	if (!QFE::FILE::HasFile(filePath)) {
		QFE_REPORT_SYSTEM_ERROR(std::string("TextureLoader: File not found - ") + filePath, SystemError::Abort);
	}

	// テクスチャファイルを読み込んでScratchImageを作成する
	DirectX::ScratchImage image{};
	std::wstring filePathW = ConvertString(filePath);
	HRESULT hr;

	// ファイルの拡張子を確認して、DDSファイルならLoadFromDDSFile、そうでなければLoadFromWICFileを呼び出す
	if (filePathW.ends_with(L".dds") || filePathW.ends_with(L".DDS")) {
		QFE_LOG(std::format("TextureLoader: Loading DDS file '{}'", filePath));
		hr = DirectX::LoadFromDDSFile(filePathW.c_str(), DirectX::DDS_FLAGS_NONE, nullptr, image);
	} else {
		QFE_LOG(std::format("TextureLoader: Loading WIC file '{}'", filePath));
		hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
	}
	assert(SUCCEEDED(hr));

#ifdef QFE_OPTIMIZE_OFF
	// 読み込んだテクスチャのメタデータをログに出力する（最適化オフのときのみ）
	const DirectX::TexMetadata& metadata = image.GetMetadata();
	QFE_LOG(std::format("TextureLoader: Loaded texture from '{}'", filePath));
	QFE_LOG(ConvertString(std::format(L"TextureLoader: width={},height={},arraySize={}", metadata.width, metadata.height, metadata.arraySize)));
#endif

	// ミップマップの生成が必要かチェック
	DirectX::ScratchImage finalImage{};
	if (image.GetMetadata().width * image.GetMetadata().height != 1) {
		if (DirectX::IsCompressed(image.GetMetadata().format)) {
			QFE_LOG(std::format("TextureLoader: Compressed texture, skipping runtime mipmap generation '{}'", filePath));
			finalImage = std::move(image);
		} else {
			QFE_LOG(std::format("TextureLoader: Generating mipmaps for texture '{}'", filePath));
			hr = DirectX::GenerateMipMaps(
				image.GetImages(), image.GetImageCount(), image.GetMetadata(),
				DirectX::TEX_FILTER_SRGB, 0, finalImage);
		}
		assert(SUCCEEDED(hr));
	} else {
		// 1x1のテクスチャはそのまま
		hr = finalImage.InitializeFromImage(*image.GetImage(0, 0, 0));
		assert(SUCCEEDED(hr));
	}

	return finalImage; // 読み込み・加工が済んだScratchImageをそのまま返す
}

DirectXResourceHandle TextureLoader::CreateResourceFromMetadata(const DirectX::TexMetadata& metadata) {
	// メタデータの内容をもとにリソース記述子を設定
	D3D12_RESOURCE_DESC resourceDesc = {};
	resourceDesc.Width = static_cast<UINT>(metadata.width); // 横幅
	resourceDesc.Height = static_cast<UINT>(metadata.height); // 縦幅
	resourceDesc.MipLevels = static_cast<UINT16>(metadata.mipLevels); // ミップマップのレベル数
	resourceDesc.DepthOrArraySize = static_cast<UINT16>(metadata.arraySize); // 配列サイズまたは深度
	resourceDesc.Format = metadata.format;// DXGI_FORMATで指定されたフォーマット
	resourceDesc.SampleDesc.Count = 1;// サンプル数（マルチサンプリングを使用しない場合は1）
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION(metadata.dimension);// リソースの次元（1D、2D、3Dなど）

	// メタデータの整合性を確認
	if (resourceDesc.Width == 0 || resourceDesc.Height == 0 || resourceDesc.MipLevels == 0 || resourceDesc.DepthOrArraySize == 0) {
		QFE_REPORT_SYSTEM_ERROR(std::string("TextureLoader: Invalid texture metadata - width, height, mipLevels, and arraySize must be greater than 0"), SystemError::Abort);
	}

	// リソースの作成
	return initializeInfo_.createResourceFunc(resourceDesc);
}

void TextureLoader::UploadTextureData(DirectXResourceHandle resourceHandle, const DirectX::ScratchImage& mipImages) {
	std::vector<D3D12_SUBRESOURCE_DATA> subresources;
	subresources.clear();
	// サブリソースデータを準備
	DirectX::PrepareUpload(initializeInfo_.device, mipImages.GetImages(), mipImages.GetImageCount(), mipImages.GetMetadata(), subresources);
	// テクスチャデータをGPUにアップロードする関数を呼び出す
	initializeInfo_.uploadTextureDataFunc(resourceHandle, subresources);
}

void QFE::GRAPHIC::TextureLoader::CreateShaderResourceView(
	const DirectX::TexMetadata& metadata, D3D12_SRV_DIMENSION texture, DirectXResourceHandle resourceHandle) {

	// メタデータを元にSRVの記述子を設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = texture;// 2Dテクスチャとして設定
	srvDesc.Texture2D.MipLevels = static_cast<UINT>(metadata.mipLevels);
	// SRVの作成
	initializeInfo_.createShaderResourceViewFunc(resourceHandle, srvDesc);
}

DirectXResourceHandle QFE::GRAPHIC::TextureLoader::CreateTextureFromScratchImage(
	DirectX::ScratchImage scratchImage, const std::string& name, D3D12_SRV_DIMENSION texture) {
	// ScratchImageからメタデータを取得
	const DirectX::TexMetadata& metadata = scratchImage.GetMetadata();
	// リソースの作成
	DirectXResourceHandle resourceHandle = CreateResourceFromMetadata(metadata);
	// SRVの作成
	CreateShaderResourceView(metadata, texture, resourceHandle);
	// データのアップロード
	UploadTextureData(resourceHandle, scratchImage);

	// マップに保存
	textureDataMap_[name] = ImageData{};
	textureDataMap_[name].resourceHandle = resourceHandle;
	textureDataMap_[name].scratchImage = std::move(scratchImage);
	return resourceHandle;
}

DirectX::ScratchImage TextureLoader::CreateDummyBlackCubeMap() {
	// 空のScratchImageを作成
	DirectX::ScratchImage scratchImage{};
	
	// 1x1サイズ、RGBA8ビット、1キューブ(6面)、ミップマップレベル1で初期化
	HRESULT hr = scratchImage.InitializeCube(DXGI_FORMAT_R8G8B8A8_UNORM, 1, 1, 1, 1);
	if (!SUCCEEDED(hr)) {
		QFE_REPORT_SYSTEM_ERROR("TextureLoader: Failed to initialize dummy cube map", SystemError::Abort);
	}

	// 全ての面のピクセルデータを0（黒）で塗りつぶす
	const DirectX::Image* images = scratchImage.GetImages();
	for (size_t i = 0; i < scratchImage.GetImageCount(); ++i) {
		memset(images[i].pixels, 0, images[i].rowPitch * images[i].height);
	}

	return scratchImage;
}

DirectX::ScratchImage TextureLoader::CreateDummyWhite1x1Texture() {
	DirectX::ScratchImage scratchImage{};

	// 普通の2Dテクスチャとして初期化する（幅1, 高1, 配列数1, ミップ1）
	HRESULT hr = scratchImage.Initialize2D(DXGI_FORMAT_R8G8B8A8_UNORM, 1, 1, 1, 1);
	if (!FAILED(hr)) { /* エラー処理 */ }

	// 1枚だけなので、images[0]（先頭）を白(255)で塗りつぶす
	const DirectX::Image* images = scratchImage.GetImages();
	std::memset(images[0].pixels, 255, images[0].rowPitch * images[0].height);

	return scratchImage; // 安全に外へ返却
}

