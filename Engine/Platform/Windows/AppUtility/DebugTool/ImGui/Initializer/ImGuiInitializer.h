#pragma once
#include <cstdint>
#include <d3d12.h>
#include "Externals/imgui/imgui.h"
#include "Externals/imgui/imgui_impl_dx12.h"
#include "Externals/imgui/imgui_impl_win32.h"              

class ImGuiInitializer final {
private:
	ImGuiInitializer() = delete;
	~ImGuiInitializer() = delete;

public:
	/// <summary>
	/// ImGuiを初期化します
	/// </summary>
	/// <param name="hwnd"></param>
	static void Initialize(
		ID3D12Device* device,uint32_t bufferCount,DXGI_FORMAT rtvFormat,
		ID3D12DescriptorHeap* srvHeap,const D3D12_CPU_DESCRIPTOR_HANDLE& cpuHandle,const D3D12_GPU_DESCRIPTOR_HANDLE& gpuHandle);
};