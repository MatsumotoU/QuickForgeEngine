/**
 * @file TextureManager.h
 * @brief テクスチャの読み込みとSRV管理を行うクラス
 */

#pragma once
#include <string>
#include <vector>
#include "engine/include/core/Math/Vector/Vector2.h"
#include "Externals/DirectXTex/d3dx12.h"
#include "Externals/DirectXTex/DirectXTex.h"

#include "engine/include/utility/String/StringLibrary.h"
#include "engine/include/core/Memory/SafeVector.h"

#include "engine/include/utility/DesignPatterns/Singleton.h"

namespace QFE {

	class SrvDescriptorHeap;

	/**
	 * @class TextureManager
	 * @brief 画像データの読み込み、リソース生成、SRVの割り当てを一括管理するシングルトンクラス
	 */
	class TextureManager final :public Singleton<TextureManager> {
		friend class Singleton<TextureManager>;
		TextureManager();
		TextureManager(const TextureManager&) = delete;
		TextureManager& operator=(const TextureManager&) = delete;
		TextureManager(TextureManager&&) = delete;
		TextureManager& operator=(TextureManager&&) = delete;

	public:
		/**
		 * @brief 初期化処理
		 * @param device DirectXデバイス
		 * @param commandList コマンドリスト
		 * @param srvDescriptorHeap SRV用ネスクリプタヒープ
		 */
		void Initialize(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, SrvDescriptorHeap* srvDescriptorHeap);

		/** @brief 終了処理 */
		void Finalize();

		/** @brief 中間リソースを解放する */
		void ReleaseIntermediateResources();

		/**
		 * @brief 画像ファイルを読み込む
		 * @param filePath ファイルパス
		 * @return テクスチャハンドル
		 */
		[[nodiscard]] int32_t LoadTexture(const std::string& filePath);

		/** @brief ハンドルからテクスチャサイズを取得 */
		[[nodiscard]] Vector2 GetTextureSize(int32_t textureHandle);
		/** @brief 指定インデックスのCPUディスクリプタハンドルを取得 */
		[[nodiscard]] const D3D12_CPU_DESCRIPTOR_HANDLE GetTextureSrvHandleCPU(uint32_t index) const;
		/** @brief 指定インデックスのGPUディスクリプタハンドルを取得 */
		[[nodiscard]] const D3D12_GPU_DESCRIPTOR_HANDLE GetTextureSrvHandleGPU(uint32_t index) const;

		/// @brief デバッグ用に次のテクスチャハンドルを取得
		[[nodiscard]] const int32_t GetNextTextureHandle() const { return textureHandle_; }

		/// @brief ダミーの黒いキューブマップのハンドルを取得
		[[nodiscard]] const int32_t GetDummyBlackCubeMapHandle() const;

	private:
		DirectX::ScratchImage Load(const std::string& filePath);
		void LoadScratchImage(const std::string& filePath);
		Microsoft::WRL::ComPtr<ID3D12Resource> CreateTextureResource(const DirectX::TexMetadata& metadata);
		void TransitionResourceBarrier(ID3D12Resource* texture, ID3D12GraphicsCommandList* commandList);
		Microsoft::WRL::ComPtr<ID3D12Resource> UploadTextureData(ID3D12Resource* texture, const DirectX::ScratchImage& mipImages, ID3D12GraphicsCommandList* commandList);
		void EndUploadTextureData(ID3D12Resource* texture, ID3D12GraphicsCommandList* commandList);
		void CreateShaderResourceView(const DirectX::TexMetadata& metadata, ID3D12Resource* textureResource);
		void CreateSkyBoxShaderResourceView(const DirectX::TexMetadata& metadata, ID3D12Resource* textureResource);

		/// @brief ダミーの黒いキューブマップを作成する
		int32_t CreateDummyBlackCubeMap();

		ID3D12Device* device_;
		ID3D12GraphicsCommandList* commandList_;
		SrvDescriptorHeap* srvDescriptorHeap_;
		D3D12_HEAP_PROPERTIES heapProperties_;
		D3D12_RESOURCE_DESC resourceDesc_;

		// 画像一枚のリソースたち
		SafeVector<D3D12_CPU_DESCRIPTOR_HANDLE> textureSrvHandleCPU_;
		SafeVector<D3D12_GPU_DESCRIPTOR_HANDLE> textureSrvHandleGPU_;
		SafeVector<Microsoft::WRL::ComPtr<ID3D12Resource>> textureResources_;
		SafeVector<std::unique_ptr<DirectX::ScratchImage>> scratchImages_;

		// ファイルパスとハンドルの対応を管理するためのライブラリ
		int32_t textureHandle_;
		SafeVector<Microsoft::WRL::ComPtr<ID3D12Resource>> intermediateResource_;
		StringLibrary filePathLibrary_;

		// エラー回避用テクスチャハンドル
		int32_t dummyBlackCubeMapHandle_;
	};

}
