#pragma once
#include "IGraphicEngine.h"
#include <memory>

#define NOMINMAX
#include <windows.h>

#include <d3d12.h>
#include <wrl.h>
#include "dx12/descriptors/Data/DescriptorHandles.h"

#include <stdint.h>
#include <string>
#include <utility>

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
		class ModelVertexResourceManager;
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
		/// @brief 画像ファイルを読み込む.テクスチャハンドルを返す.
		uint32_t LoadTexture(const std::string& filePath);
		/// @brief モデルデータからテクスチャハンドルと頂点バッファハンドルを作成する.モデルデータはAssimpで読み込める形式であれば何でもいい.
		/// @return textureHandle,vertexBufferHandleの順で返す.
		std::pair<uint32_t, uint32_t> LoadModel(const std::string& filePath);

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
		std::unique_ptr<INTERNAL::ModelVertexResourceManager> modelVertexResourceManager_;// モデル頂点リソース管理クラス

		D3D12_VIEWPORT viewport_;
		D3D12_RECT scissorRect_;

		Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilBuffer_;
		INTERNAL::DescriptorHandles dsvHandle_;
	};
}