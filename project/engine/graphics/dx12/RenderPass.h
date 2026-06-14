#pragma once
#define NOMINMAX
#include <windows.h>
#include <wrl.h>
#include <dxgi1_6.h>
#include <d3d12.h>

#include <memory>
#include <vector>
#include <functional>

#include "descriptors/Data/DescriptorHandles.h"
#include "GraphicEngineHandleTypes.h"

namespace QFE::GRAPHIC::INTERNAL {
	class SwapChain;
	class OffscreenBuffer;

	/// @brief 描画パスの初期化に必要な情報をまとめた構造体
	struct RenderPassInitializeInfo {
		uint32_t width;
		uint32_t height;
		HWND hwnd;
		ID3D12Device* device;
		IDXGIFactory7* dxgiFactory;
		ID3D12CommandQueue* commandQueue;
		std::function<DescriptorHandles(ID3D12Resource*, const D3D12_RENDER_TARGET_VIEW_DESC*)> assignRtvFunc;// Rtvを割り当てる関数
		std::function<DescriptorHandles(ID3D12Resource*, const D3D12_SHADER_RESOURCE_VIEW_DESC*)> assignSrvFunc;// Srvを割り当てる関数
	};

	/// @brief 描画先のバリア、管理するクラス
	class RenderPass final {
	public:
		RenderPass();
		~RenderPass();

	public:
		/// @brief 描画先のリソースを生成し、描画先のバリアを管理するクラスを初期化します。
		void Initialize(const RenderPassInitializeInfo& initializeInfo);
		/// @brief 描画先のリソースを描画用に変更するバリアを発行します。
		void PreDraw(ID3D12GraphicsCommandList* commandList);
		/// @brief 描画先のリソースを読み込み用に変更するバリアを発行します。
		void PostDraw(ID3D12GraphicsCommandList* commandList);
		/// @brief バックバッファを画面に表示します。
		void Present();

		/// @brief 描画先を決定します。0はスワップチェーンのバックバッファ、1以上はオフスクリーンバッファを指します。
		void SetRenderTarget(ID3D12GraphicsCommandList* commandList, RenderTargetHandle renderTargetHandle = RenderTargetHandle::SwapChain);

	private:
		/// @brief 描画先のリソースを描画用に変更するバリアを発行します。
		void TransitionRenderTargetToRenderTarget(ID3D12GraphicsCommandList* commandList);
		/// @brief 描画先のリソースを読み込み用に変更するバリアを発行します。
		void TransitionRenderTargetToPresent(ID3D12GraphicsCommandList* commandList);
		/// @brief 全ての描画先をクリアします。
		void ClearRenderTarget(ID3D12GraphicsCommandList* commandList, const float clearColor[4]);

		std::unique_ptr<SwapChain> swapChain_;
		std::unique_ptr<OffscreenBuffer> offscreenBuffer_;

		std::vector<uint32_t> offscreenHandles_;
	};
}