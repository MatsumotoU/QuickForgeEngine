#pragma once
#define NOMINMAX
#include <windows.h>
#include <wrl.h>
#include <dxgi1_6.h>
#include <d3d12.h>
#include "Descriptors/Data/DescriptorHandles.h"

#include "memory/SafeVector.h"

namespace QFE::GRAPHIC::INTERNAL {
	/// @brief DirectX12のスワップチェーンを管理するクラス
	class SwapChain {
	public:
		SwapChain() = default;
		~SwapChain() = default;
		/// @brief スワップチェーンの初期化
		void Initialize(HWND hwnd, uint32_t width, uint32_t height, IDXGIFactory7* dxgiFactory, ID3D12CommandQueue* commandQueue);
		/// @brief スワップチェーンのリソースをある色でクリアする
		void ClearBackBuffer(ID3D12GraphicsCommandList* commandList, const FLOAT clearColor[4]);

		/// @brief 現在のバックバッファを描画可能に変更
		void TransitionCurrentBackBufferToRenderTarget(ID3D12GraphicsCommandList* commandList);
		/// @brief 現在のバックバッファを読み込み可能に変更
		void TransitionCurrentBackBufferToPresent(ID3D12GraphicsCommandList* commandList);

		ID3D12Resource* GetCurrentBackBuffer() const;
		D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentBackBufferView() const;
		const D3D12_CPU_DESCRIPTOR_HANDLE* GetCurrentBackBufferViewPtr() const;
		uint32_t GetCurrentBackBufferIndex() const;
		ID3D12Resource* GetBackBuffer(uint32_t index) const;
		uint32_t GetBackBufferCount() const;

		void CreateDubleBuffering();

		void Present();
		void AssignDescriptorHandles(const DescriptorHandles& rtvHandle, uint32_t index);
		/// <summary>
		/// バックバッファとビューの数が一致しているか確認
		/// </summary>
		/// <returns></returns>
		bool CheckBackBufferViews() const;

	private:
		void CreateSwapChain(HWND hwnd, uint32_t width, uint32_t height, IDXGIFactory7* dxgiFactory, ID3D12CommandQueue* commandQueue);
		void AssignBackbuffer();

		Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain_;
		SafeVector<Microsoft::WRL::ComPtr<ID3D12Resource>> backBuffers_;
		SafeVector<DescriptorHandles> backBufferViews_;

		uint32_t rtvDescriptorSize_;
		uint32_t currentBackBufferIndex_;
	};

}
