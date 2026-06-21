#pragma once
#include "IGraphicEngine.h"
#include "dx12/vram/descriptors/DescriptorHandles.h"
#include "dx12/GraphicEngineHandleTypes.h"

#include "memory/UniqueContainer.h"
#include "../resources/Shaders/ShaderStructs/hlslTypeToCpp.h"

#define NOMINMAX
#include <windows.h>
#include <d3d12.h>
#include <wrl.h>
#include <stdint.h>
#include <string>
#include <utility>
#include <vector>
#include <memory>

namespace QFE::GRAPHIC {
	/// @brief DirectX12のグラフィックエンジンの実装クラスで使用しているクラス
	class DirectX12DebugCore;
	class DirectXDevice;
	class DirectXResourceContainer;
	class DescriptorHeapManager;
	class DirectXCommandManager;
	class Fence;
	class ShaderCompiler;
	class GraphicPipelineManager;
	class TextureLoader;
	class RenderPass;

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

		/// @brief ビューポートの作成.
		ViewPortHandle CreateViewPort(uint32_t width, uint32_t height);
		/// @brief シザリング矩形の作成.
		ScissorRectHandle CreateScissorRect(int left, int top, int right, int bottom);

		// ユーザーが任意のタイミングで呼び出す関数群
		/// @brief 画像ファイルを読み込む.
		DirectXResourceHandle LoadTexture(const std::string& filePath);
		/// @brief 頂点データから頂点バッファハンドルを作成する.
		DirectXResourceHandle LoadMesh(const std::vector<VertexData>& vertexData, const std::string& meshName);

		void TestDraw(
			ViewPortHandle viewportHandle, ScissorRectHandle scissorRectHandle,
			DirectXResourceHandle vertexBufferHandle);

	private:
		/// @brief DirectXCommonの名残.fenceの初期化以降の処理.
		void LegacyInitialize(uint32_t width, uint32_t height);
		DirectXResourceHandle CreateDepthStencilBuffer(uint32_t width, uint32_t height);
		void ClearDepthStencil(DirectXResourceHandle depthStencilHandle);

		HWND hwnd_;// ウィンドウハンドル
		std::unique_ptr<DirectX12DebugCore> debugCore_;// DirectX12のデバッグコアクラス

		std::unique_ptr<DirectXDevice> directXDevice_;// DirectX12の共通管理クラス
		std::unique_ptr<DirectXResourceContainer> resourceContainer_;// DirectX12のリソース管理クラス
		std::unique_ptr<DescriptorHeapManager> descriptorHeapManager_;// デスクリプタヒープ管理クラス
		std::unique_ptr<DirectXCommandManager> commandManager_;// コマンド管理クラス
		std::unique_ptr<RenderPass> renderPass_;// 描画先管理クラス
		std::unique_ptr<Fence> fence_;// フェンス管理クラス

		std::unique_ptr<ShaderCompiler> shaderCompiler_;// シェーダーコンパイルクラス
		std::unique_ptr<GraphicPipelineManager> graphicPipelineManager_;// グラフィックパイプライン管理クラス

		std::unique_ptr<TextureLoader> textureLoader_;// テクスチャ管理クラス

		QFE::UniqueContainer<D3D12_VIEWPORT> viewports_;// ビューポートのコンテナ
		QFE::UniqueContainer<D3D12_RECT> scissorRects_;// シザリング矩形のコンテナ

		DirectXResourceHandle depthStencilBufferHandle_;// 深度ステンシルバッファのリソースハンドル
	};
}