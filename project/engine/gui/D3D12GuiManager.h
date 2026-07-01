#pragma once
#include "IGuiManager.h"

#define NOMINMAX
#include <Windows.h>
#include <d3d12.h>

namespace QFE::GUI {
	/// @brief D3D12GuiManagerの初期化用構造体
	struct D3D12GuiManagerInitDesc {
		ID3D12Device* device = nullptr; // D3D12デバイス
		ID3D12GraphicsCommandList* commandList = nullptr; // D3D12コマンドリスト
		UINT bufferCount = 2; // バッファ数
		DXGI_FORMAT rtvFormat = DXGI_FORMAT_R8G8B8A8_UNORM; // レンダーターゲットのフォーマット
		ID3D12DescriptorHeap* srvHeap = nullptr; // SRVヒープ
		D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = {}; // SRVのCPUハンドル
		D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = {}; // SRVのGPUハンドル
		HWND hwnd = nullptr; // ウィンドウハンドル
	};

	/// @brief ImGuiを使用したGUIマネージャー
	class D3D12GuiManager : public IGuiManager {
	public:
		D3D12GuiManager() = default;
		virtual ~D3D12GuiManager() = default;

		/// @brief 初期化処理
		void Initialize(const D3D12GuiManagerInitDesc& desc);
		/// @brief 更新処理
		void Update() override;
		/// @brief 描画前処理
		void PreDraw() override;

		/// @brief 描画後処理
		void PostDraw() override;
		/// @brief 終了処理
		void Shutdown() override;

	private:
		ID3D12GraphicsCommandList* commandList_; // D3D12コマンドリスト
	};
}