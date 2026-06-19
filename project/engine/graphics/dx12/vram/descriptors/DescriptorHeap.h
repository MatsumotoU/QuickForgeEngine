#pragma once
#include <wrl.h>
#include <d3d12.h>
#include <queue>
#include "Data/DescriptorHeapInfo.h"
#include "Data/DescriptorHandles.h"

namespace QFE::GRAPHIC::INTERNAL {
	/// @brief ディスクリプターヒープ
	class DescriptorHeap {
	public:
		/// @brief ディスクリタヒープを生成します
		void Create(ID3D12Device* device, UINT numDescriptors, bool shaderVisible);

		[[nodiscard]] ID3D12DescriptorHeap* GetDescriptorHeap() const;
		[[nodiscard]] ID3D12DescriptorHeap* const* GetDescriptorHeapAddressOf() const;
		[[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandleForHeapStart() const;
		[[nodiscard]] D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandleForHeapStart() const;
	protected:
		ID3D12Device* device_ = nullptr;
		D3D12_DESCRIPTOR_HEAP_DESC DescriptorHeapInfo_{};
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap_;
		std::queue<UINT> freeDescriptors_;
	};
}