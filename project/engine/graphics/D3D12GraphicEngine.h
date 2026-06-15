#pragma once
#include "IGraphicEngine.h"

#include "memory/UniqueContainer.h"

#include <d3d12.h>
#include <wrl.h>
#include "dx12/descriptors/Data/DescriptorHandles.h"
#include "common/ModelData.h"

#define NOMINMAX
#include <windows.h>
#include <stdint.h>
#include <string>
#include <utility>
#include <vector>
#include <memory>

#include "dx12/GraphicEngineHandleTypes.h"

namespace QFE::GRAPHIC {
	/// @brief DirectX12のグラフィックエンジンの実装クラスで使用しているクラス
	namespace INTERNAL {
		class DirectX12DebugCore;
		class DirectXDevice;
		class DirectXResourceContainer;
		class DescriptorHeapManager;
		class DirectXCommandManager;
		class SwapChain;
		class Fence;

		class ShaderCompiler;

		class GraphicPipelineManager;
		class ModelDataContainer;
		class VertexBufferContainer;
		class TextureLoader;
		class RenderPass;
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
		TextureHandle LoadTexture(const std::string& filePath);
		/// @brief モデルデータを読み込む.モデルデータから頂点バッファリソースを作成し、ハンドルを返す.
		ModelHandle LoadModel(const std::string& filePath);
		/// @brief 頂点データから頂点バッファハンドルを作成する.
		VertexBufferHandle LoadMesh(const std::vector<VertexData>& vertexData, const std::string& meshName);
		/// @brief モデルデータのハンドルからモデルのメッシュの頂点バッファハンドルを取得する.
		std::vector<VertexBufferHandle> LoadMeshesFromModel(ModelHandle modelHandle);
		/// @brief ビューポートの作成.
		ViewPortHandle CreateViewPort(uint32_t width, uint32_t height);
		/// @brief シザリング矩形の作成.
		ScissorRectHandle CreateScissorRect(int left, int top, int right, int bottom);

	private:
		/// @brief DirectXCommonの名残.fenceの初期化以降の処理.
		void LegacyInitialize(uint32_t width, uint32_t height);
		INTERNAL::DirectXResourceHandle CreateDepthStencilBuffer(uint32_t width, uint32_t height);
		void ClearDepthStencil(INTERNAL::DirectXResourceHandle depthStencilHandle);

		HWND hwnd_;// ウィンドウハンドル
		std::unique_ptr<INTERNAL::DirectX12DebugCore> debugCore_;// DirectX12のデバッグコアクラス

		std::unique_ptr<INTERNAL::DirectXDevice> directXDevice_;// DirectX12の共通管理クラス
		std::unique_ptr<INTERNAL::DirectXResourceContainer> resourceContainer_;// DirectX12のリソース管理クラス
		std::unique_ptr<INTERNAL::DescriptorHeapManager> descriptorHeapManager_;// デスクリプタヒープ管理クラス
		std::unique_ptr<INTERNAL::DirectXCommandManager> commandManager_;// コマンド管理クラス
		std::unique_ptr<INTERNAL::RenderPass> renderPass_;// 描画先管理クラス
		std::unique_ptr<INTERNAL::Fence> fence_;// フェンス管理クラス

		std::unique_ptr<INTERNAL::ShaderCompiler> shaderCompiler_;// シェーダーコンパイルクラス

		std::unique_ptr<INTERNAL::GraphicPipelineManager> graphicPipelineManager_;// グラフィックパイプライン管理クラス
		std::unique_ptr<INTERNAL::TextureLoader> textureLoader_;// テクスチャ管理クラス
		std::unique_ptr<INTERNAL::ModelDataContainer> modelDataContainer_;// モデル頂点リソース管理クラス
		std::unique_ptr<INTERNAL::VertexBufferContainer> vertexBufferContainer_;// 頂点バッファ管理クラス

		QFE::UniqueContainer<D3D12_VIEWPORT> viewports_;// ビューポートのコンテナ
		QFE::UniqueContainer<D3D12_RECT> scissorRects_;// シザリング矩形のコンテナ

		INTERNAL::DirectXResourceHandle depthStencilBufferHandle_;// 深度ステンシルバッファのリソースハンドル
	};
}