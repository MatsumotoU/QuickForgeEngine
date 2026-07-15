/**
 * @file TextureLoader.h
 * @brief テクスチャの読み込みとSRV管理を行うクラス
 */

#pragma once
#include <string>
#include <vector>
#include <functional>
#include <unordered_map>
#include <d3d12.h>

#include "DirectXTex/d3dx12.h"
#include "DirectXTex/DirectXTex.h"

#include "math/vector/Vector2.h"
#include "dx12/vram/descriptors/DescriptorHandles.h"

#include "GraphicEngineHandleTypes.h"

namespace QFE::GRAPHIC {
	/// @brief TextureLoaderクラスの初期化に必要な情報と関数をまとめた構造体
	struct TextureLoaderInitializeInfo {
		ID3D12Device* device;
		std::function<DirectXResourceHandle(const D3D12_RESOURCE_DESC&)> createResourceFunc;// テクスチャリソースを作成する関数
		std::function<void(DirectXResourceHandle, const D3D12_SHADER_RESOURCE_VIEW_DESC&)> createShaderResourceViewFunc;// シェーダーリソースビューを作成する関数
		std::function<void(DirectXResourceHandle,const std::vector<D3D12_SUBRESOURCE_DATA>&)> uploadTextureDataFunc;// テクスチャデータをGPUにアップロードする関数
	};

	/// @brief このクラスで保持する画像データ
	struct ImageData {
		DirectXResourceHandle resourceHandle = DirectXResourceHandle::Invalid;// 結びついているテクスチャリソースのハンドル
		DirectX::ScratchImage scratchImage;
	};

	/// @brief テクスチャの読み込み、リソースのハンドルとテクスチャ名を結び付けて保存するクラス
	class TextureLoader final {
	public:
		/// @brief 初期化処理
		void Initialize(TextureLoaderInitializeInfo info);
		/** @brief 終了処理 */
		void Finalize();

		/// @brief 画像ファイルを読み込む、返り値はリソースハンドル
		[[nodiscard]] DirectXResourceHandle LoadTexture(const std::string& filePath);
		/// @brief ダミーの黒いキューブマップのハンドルを取得
		[[nodiscard]] const DirectXResourceHandle GetDummyBlackCubeMapHandle() const;
		/// @brief ダミーの白い1x1テクスチャのハンドルを取得
		[[nodiscard]] const DirectXResourceHandle GetDummyWhite1x1TextureHandle() const;

	private:
		/// @brief 画像ファイルを読み込んでDirectX::ScratchImageを作成する関数
		DirectX::ScratchImage LoadScratchImageFromFile(const std::string& filePath);
		/// @brief metadataを元にテクスチャリソースを作成し、リソースハンドルを返す関数
		DirectXResourceHandle CreateResourceFromMetadata(const DirectX::TexMetadata& metadata);
		/// @brief テクスチャデータをGPUにアップロードする関数
		void UploadTextureData(DirectXResourceHandle resourceHandle,const DirectX::ScratchImage& mipImages);
		/// @brief metadataを元にSRVを作成する関数
		void CreateShaderResourceView(
			const DirectX::TexMetadata& metadata, D3D12_SRV_DIMENSION texture, DirectXResourceHandle resourceHandle);
		/// @brief ScratchImageと名前からテクスチャとして使えるようにする関数。SRVにも登録する
		DirectXResourceHandle CreateTextureFromScratchImage(
			DirectX::ScratchImage scratchImage, const std::string& name, D3D12_SRV_DIMENSION texture);

		/// @brief ダミーの黒いキューブマップを作成する
		DirectX::ScratchImage CreateDummyBlackCubeMap();
		/// @brief ダミーの白い1x1テクスチャを作成する
		DirectX::ScratchImage CreateDummyWhite1x1Texture();

		TextureLoaderInitializeInfo initializeInfo_;
		std::unordered_map<std::string, ImageData> textureDataMap_;// ファイル名と画像データの対応を管理するマップ
	};

}
