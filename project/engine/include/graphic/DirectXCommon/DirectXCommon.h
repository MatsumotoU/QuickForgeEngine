/**
 * @file DirectXCommon.h
 * @brief DirectX12の共通基盤管理クラス
 */

#pragma once
#include "engine/include/core/Graphic/IGraphicCommon.h"
#include "engine/include/utility/DesignPatterns/Singleton.h"

#include "Descriptors/DescriptorHeapManager.h"

#include "DirectXDevice.h"
#include "Command/DirectXCommandManager.h"
#include "SwapChain.h"
#include "Fence.h"

#include "engine/include/utility/DebugTool/DirectX/DirectX12DebugCore.h" 

namespace QFE {

	class GameWindow;
	class DirectXDevice;

	/**
	 * @class DirectXCommon
	 * @brief DirectX12のデバイス、コマンド、スワップチェーンなどを一括管理する管理クラス
	 */
	class DirectXCommon final {
	public:
		static DirectXCommon* GetInstance();

		/**
		 * @brief DirectX12の初期化
		 * @param gameWindow ウィンドウマネージャーから取得したウィンドウポインタ
		 * @param width 横幅
		 * @param height 縦幅
		 */
		void Initialize(const HWND& hwnd, uint32_t width, uint32_t height);
		/** @brief 描画前処理 */
		void PreDraw();
		/** @brief 描画後処理 */
		void PostDraw();
		/** @brief 終了処理 */
		void Shutdown();

		/** @brief レンダーターゲットビュー用デスクリプタヒープのアサイン */
		[[nodiscard]] DescriptorHandles AssignRtvHeap(ID3D12Resource* resource, const D3D12_RENDER_TARGET_VIEW_DESC* desc);
		/** @brief シェーダーリソースビュー用デスクリプタヒープのアサイン */
		[[nodiscard]] DescriptorHandles AssignSrvHeap(ID3D12Resource* resource, const D3D12_SHADER_RESOURCE_VIEW_DESC& desc);
		/** @brief デプスステンシルビュー用デスクリプタヒープのアサイン */
		[[nodiscard]] DescriptorHandles AssignDsvHeap(ID3D12Resource* resource, const D3D12_DEPTH_STENCIL_VIEW_DESC* desc);

		void ClearDepthStencil();

		ID3D12Device* GetDevice();
		ID3D12GraphicsCommandList* GetCommandManager(const D3D12_COMMAND_LIST_TYPE& type);
		SwapChain* GetSwapChain();
		uint32_t GetBackBufferCount();
		Fence* GetFence();
		D3D12_RENDER_TARGET_VIEW_DESC& GetSwapChainRtvDesc();
		DescriptorHeapManager* GetDescriptorHeapManager();
		ID3D12DescriptorHeap* GetRtvDescriptorHeapAddress();
		ID3D12DescriptorHeap* GetSrvDescriptorHeapAddress();
		ID3D12DescriptorHeap* GetDsvDescriptorHeapAddress();
		ID3D12DescriptorHeap* const* GetRtvDescriptorHeapAddressOf();
		ID3D12DescriptorHeap* const* GetSrvDescriptorHeapAddressOf();
		ID3D12DescriptorHeap* const* GetDsvDescriptorHeapAddressOf();
		D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentBackBufferCpuHandle();

		DescriptorHandles* GetDepthStencilViewHandle();
		D3D12_DEPTH_STENCIL_DESC GetDepthStencilDesc();
		D3D12_VIEWPORT* GetViewPort();
		D3D12_RECT* GetScissorRect();

	private:
		std::unique_ptr<DirectX12DebugCore> debugCore_;

		void AssignSwapChainRenderTarget();

		DescriptorHeapManager descriptorHeapManager_;
		DirectXDevice directXDevice_;
		DirectXCommandManager commandManager_;
		SwapChain swapChain_;
		Fence fence_;
		D3D12_VIEWPORT viewport_;
		D3D12_RECT scissorRect_;

		Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilBuffer_;
		DescriptorHandles dsvHandle_;

		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc_;
	};

}
