#pragma once
#define NOMINMAX
#include <windows.h>
#include <wrl.h>
#include <dxgi1_6.h>
#include <d3d12.h>

#include <memory>
#include <vector>
#include <functional>

#include "GraphicEngineHandleTypes.h"

namespace QFE::GRAPHIC {
	class SwapChain;

	/// @brief 描画パスの初期化に必要な情報をまとめた構造体
	struct RenderPassInitializeInfo {
		uint32_t width;
		uint32_t height;
		HWND hwnd;
		ID3D12Device* device;
		IDXGIFactory7* dxgiFactory;
		ID3D12CommandQueue* commandQueue;

		std::function<DirectXResourceHandle(uint32_t, uint32_t, DXGI_FORMAT)> createOffscreenFunc;

		std::function<D3D12_CPU_DESCRIPTOR_HANDLE(ID3D12Resource*, const D3D12_RENDER_TARGET_VIEW_DESC*)>assginRtvFunc;
		std::function<const D3D12_CPU_DESCRIPTOR_HANDLE*(DirectXResourceHandle)>getResourceDsvFunc;// Dsvをリソースハンドルから取得する関数
		std::function<bool(DirectXResourceHandle, D3D12_RESOURCE_STATES, D3D12_RESOURCE_STATES)> transitionFunc;// リソースの状態を変更する関数
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

		/// @brief オフスクリーンのレンダーターゲットを生成します。戻り値はRenderTargetHandleです。
		RenderTargetHandle CreateOffscreenRenderTarget(uint32_t width, uint32_t height, DXGI_FORMAT format);

		/// @brief 描画先を決定します。0はスワップチェーンのバックバッファ、1以上はオフスクリーンバッファを指します。
		void SetRenderTarget(
			ID3D12GraphicsCommandList* commandList, DirectXResourceHandle depthStencilHandle,
			RenderTargetHandle renderTargetHandle = RenderTargetHandle::SwapChain);
		/// @brief 描画先を決定します。0はスワップチェーンのバックバッファ、1以上はオフスクリーンバッファを指します。
		void SetRenderTarget(
			ID3D12GraphicsCommandList* commandList, DirectXResourceHandle depthStencilHandle,
			std::vector<RenderTargetHandle> renderTargetHandles);

		/// @brief スワップチェーンのバックバッファの数を取得します。
		UINT GetSwapChainBufferCount() const;


		void TransitionCurrentBackBufferBarrier(ID3D12GraphicsCommandList* commandList, D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState);
		ID3D12Resource* GetCurrentBackBuffer() const;

		/// @brief RenderTargetHandleからDirectXResourceHandleを取得します。
		DirectXResourceHandle GetRenderTargetResourceHandle(RenderTargetHandle renderTargetHandle) const;

	private:
		std::unique_ptr<SwapChain> swapChain_;
		RenderPassInitializeInfo initializeInfo_;

		// OffscreenRenderTargetのハンドルを管理するための配列
		std::vector<DirectXResourceHandle> offscreenRenderTargetsHandle_;
	};
}