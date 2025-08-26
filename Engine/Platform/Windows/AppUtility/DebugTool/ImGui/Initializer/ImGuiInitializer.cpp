#include "ImGuiInitializer.h"
#include <cassert>

void ImGuiInitializer::Initialize(
	ID3D12Device* device, uint32_t bufferCount, DXGI_FORMAT rtvFormat,
	ID3D12DescriptorHeap* srvHeap, const D3D12_CPU_DESCRIPTOR_HANDLE& cpuHandle, const D3D12_GPU_DESCRIPTOR_HANDLE& gpuHandle) {
	bool isInit = ImGui_ImplDX12_Init(device,
		bufferCount,
		rtvFormat,
		srvHeap,
		cpuHandle,
		gpuHandle);
	assert(isInit);
}
