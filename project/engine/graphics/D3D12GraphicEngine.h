#pragma once
#include "IGraphicEngine.h"
#include <memory>

#define NOMINMAX
#include <windows.h>

namespace QFE::GRAPHIC {
	namespace INTERNAL {
		class DirectXDevice;
		class DirectXResourceContainer;
		class DescriptorHeapManager;
		class DirectXCommandManager;
		class SwapChain;
		class Fence;
	}

	/// @brief DirectX12を使用したグラフィックエンジンの実装クラス
	class D3D12GraphicEngine final : public IGraphicEngine {
	public:
		/// @brief wndowsに依存したグラフィックエンジンです.描画ウィンドウのハンドルの引数に取ります.
		explicit D3D12GraphicEngine(HWND hwnd);

		void Initialize() override;
		void PreDraw() override;
		void PostDraw() override;
		void Shutdown() override;

	private:
		HWND hwnd_;// ウィンドウハンドル

		std::unique_ptr<INTERNAL::DirectXDevice> directXCommon_;// DirectX12の共通管理クラス
		std::unique_ptr<INTERNAL::DirectXResourceContainer> resourceContainer_;// DirectX12のリソース管理クラス
		std::unique_ptr<INTERNAL::DescriptorHeapManager> descriptorHeapManager_;// デスクリプタヒープ管理クラス
		std::unique_ptr<INTERNAL::DirectXCommandManager> commandManager_;// コマンド管理クラス
		std::unique_ptr<INTERNAL::SwapChain> swapChain_;// スワップチェーン管理クラス
		std::unique_ptr<INTERNAL::Fence> fence_;// フェンス管理クラス
	};
}