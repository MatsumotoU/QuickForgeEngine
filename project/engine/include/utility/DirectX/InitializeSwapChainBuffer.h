#pragma once
#include <d3d12.h>

namespace QFE {
	class InitializeSwapChainBuffer final {
	public:
		static void Execute(
			ID3D12GraphicsCommandList* list, const D3D12_CPU_DESCRIPTOR_HANDLE& handle, float red, float green, float blue, float alpha);

	private:
		InitializeSwapChainBuffer() = delete;
		~InitializeSwapChainBuffer() = delete;
	};
}