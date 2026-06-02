#pragma once
#include "IGraphicEngine.h"
#include <memory>

#define NOMINMAX
#include <windows.h>

#include <d3d12.h>
#include <wrl.h>
#include "dx12/descriptors/Data/DescriptorHandles.h"
#include "common/ModelData.h"

#include <stdint.h>
#include <string>
#include <utility>
#include <vector>

namespace QFE::GRAPHIC {
	namespace INTERNAL {
		class DirectX12DebugCore;
		class DirectXDevice;
		class DirectXResourceContainer;
		class DescriptorHeapManager;
		class DirectXCommandManager;
		class SwapChain;
		class Fence;

		class GraphicPipelineManager;
		class ModelDataContainer;
		class VertexBufferContainer;
		class TextureManager;
	}

	/// @brief DirectX12を使用したグラフィックエンジンの実装クラス
	class D3D12GraphicEngine final : public IGraphicEngine {
	public:
		/// @brief wndowsに依存したグラフィックエンジンです.描画ウィンドウのハンドルの引数に取ります.
		explicit D3D12GraphicEngine(HWND hwnd);
		~D3D12GraphicEngine() override;

		// 一度は呼ぶ順番がある関数群
		void Initialize() override;
		void PreDraw() override;
		void Draw() override {};
		void PostDraw() override;
		void Shutdown() override;

		// ユーザーが任意のタイミングで呼び出す関数群
		/// @brief 画像ファイルを読み込む.
		/// @return 読み込んだテクスチャのハンドルを返します.読み込みに失敗した場合はUINT32_MAXを返します.
		uint32_t LoadTexture(const std::string& filePath);
		/// @brief モデルデータを読み込む.モデルデータから頂点バッファリソースを作成し、ハンドルを返す.
		/// @return モデルデータのハンドルを返します.読み込みに失敗した場合はUINT32_MAXを返します.
		uint32_t LoadModel(const std::string& filePath);
		/// @brief 頂点データから頂点バッファハンドルを作成する.
		/// @return 頂点バッファのハンドルを返します.読み込みに失敗した場合はUINT32_MAXを返します.
		uint32_t LoadMesh(const std::vector<VertexData>& vertexData, const std::string& meshName);
		/// @brief モデルデータのハンドルからモデルのメッシュの頂点バッファハンドルを取得する.
		/// @return 頂点データのハンドル配列(モデルのメッシュ順).読み込み失敗した場合は空の配列を返します.
		std::vector<uint32_t> LoadMeshesFromModel(uint32_t modelHandle);

	private:
		/// @brief DirectXCommonの名残.fenceの初期化以降の処理.
		void LegacyInitialize(uint32_t width, uint32_t height);
		void AssignSwapChainDescriptor();
		void CreateDepthStencilBuffer(uint32_t width, uint32_t height);
		void CreateViewportAndScissorRect(uint32_t width, uint32_t height);
		void ClearDepthStencil();

		HWND hwnd_;// ウィンドウハンドル
		std::unique_ptr<INTERNAL::DirectX12DebugCore> debugCore_;// DirectX12のデバッグコアクラス

		std::unique_ptr<INTERNAL::DirectXDevice> directXDevice_;// DirectX12の共通管理クラス
		std::unique_ptr<INTERNAL::DirectXResourceContainer> resourceContainer_;// DirectX12のリソース管理クラス
		std::unique_ptr<INTERNAL::DescriptorHeapManager> descriptorHeapManager_;// デスクリプタヒープ管理クラス
		std::unique_ptr<INTERNAL::DirectXCommandManager> commandManager_;// コマンド管理クラス
		std::unique_ptr<INTERNAL::SwapChain> swapChain_;// スワップチェーン管理クラス
		std::unique_ptr<INTERNAL::Fence> fence_;// フェンス管理クラス

		std::unique_ptr<INTERNAL::GraphicPipelineManager> graphicPipelineManager_;// グラフィックパイプライン管理クラス
		std::unique_ptr<INTERNAL::TextureManager> textureManager_;// テクスチャ管理クラス
		std::unique_ptr<INTERNAL::ModelDataContainer> modelDataContainer_;// モデル頂点リソース管理クラス
		std::unique_ptr<INTERNAL::VertexBufferContainer> vertexBufferContainer_;// 頂点バッファ管理クラス

		D3D12_VIEWPORT viewport_;
		D3D12_RECT scissorRect_;

		Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilBuffer_;
		INTERNAL::DescriptorHandles dsvHandle_;
	};
}