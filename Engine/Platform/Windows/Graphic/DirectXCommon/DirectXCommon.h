#pragma once
#include "Core/Graphic/IGraphicCommon.h"

#include "Descriptors/DescriptorHeapManager.h"

#include "Device/DirectXDevice.h"
#include "Command/DirectXCommandManager.h"
#include "SwapChain/SwapChain.h"
#include "Fence/Fence.h"

#ifdef _DEBUG
#include "AppUtility/DebugTool/DirectX/DirectX12DebugCore.h"
#endif // _DEBUG

class DirectXCommon final {
public:
	DirectXCommon() = default;
	~DirectXCommon() = default;
	void Initialize(HWND hwnd, uint32_t width, uint32_t height);
	void PreDraw();
	void PostDraw();
	void Shutdown();

	ID3D12Device* GetDevice();
	ID3D12GraphicsCommandList* GetCommandManager(const D3D12_COMMAND_LIST_TYPE& type);
	SwapChain* GetSwapChain();
	Fence* GetFence();
	ID3D12DescriptorHeap* GetRtvDescriptorHeapAddress();
	ID3D12DescriptorHeap* GetSrvDescriptorHeapAddress();
	ID3D12DescriptorHeap* const* GetRtvDescriptorHeapAddressOf();
	ID3D12DescriptorHeap* const* GetSrvDescriptorHeapAddressOf();

private:
#ifdef _DEBUG
	DirectX12DebugCore directX12DebugCore_;
#endif // _DEBUG

	void AssignSwapChainRenderTarget();

	DescriptorHeapManager descriptorHeapManager_;
	DirectXDevice directXDevice_;
	DirectXCommandManager commandManager_;
	SwapChain swapChain_;
	Fence fence_;
};