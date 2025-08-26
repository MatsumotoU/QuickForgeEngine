#pragma once
#include <windows.h>
#include <wrl.h>
#include <dxgi1_6.h>
#include <d3d12.h>
#include <vector>
#include "../Descriptors/Data/DescriptorHandles.h"

class SwapChain {
public:
	SwapChain() = default;
	~SwapChain() = default;
	void Initialize(HWND hwnd, uint32_t width, uint32_t height, IDXGIFactory7* dxgiFactory, ID3D12CommandQueue* commandQueue);
	
	ID3D12Resource* GetCurrentBackBuffer() const;
	D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentBackBufferView() const;
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
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> backBuffers_;
	std::vector<DescriptorHandles> backBufferViews_;

	uint32_t rtvDescriptorSize_;
	uint32_t currentBackBufferIndex_;
};