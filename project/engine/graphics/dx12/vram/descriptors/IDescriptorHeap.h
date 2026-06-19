#pragma once
#include <wrl.h>
#include <d3d12.h>
#include <queue>
#include "Data/DescriptorHeapInfo.h"
#include "Data/DescriptorHandles.h"

namespace QFE::GRAPHIC::INTERNAL {
	class IDescriptorHeap {
	public:
		virtual ~IDescriptorHeap() = default;
		virtual void Initialize(ID3D12Device* device, UINT numDescriptors, bool shaderVisible) = 0;
		[[nodiscard]] ID3D12DescriptorHeap* GetDescriptorHeap() const;
		[[nodiscard]] ID3D12DescriptorHeap* const* GetDescriptorHeapAddressOf() const;
		[[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandleForHeapStart() const;
		[[nodiscard]] D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandleForHeapStart() const;

		/// @brief ある位置のディスクリタハンドルを取得する
		[[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE GetCpuDescriptorHandle(uint32_t index) const;
		/// @brief ある位置のディスクリタハンドルを取得する
		[[nodiscard]] D3D12_GPU_DESCRIPTOR_HANDLE GetGpuDescriptorHandle(uint32_t index) const;

		virtual UINT GetDescriptorSize() const = 0;

	protected:
		ID3D12Device* device_;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap_;
		std::queue<uint32_t> freeDescriptors_;
		DescriptorHeapInfo DescriptorHeapInfo_;
	};
}