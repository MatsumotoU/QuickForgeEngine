#pragma once
#include "Core/Graphic/IGraphicCommon.h"
#include "Utility/DesignPatterns/Singleton.h"

#include "Descriptors/DescriptorHeapManager.h"

#include "Device/DirectXDevice.h"
#include "Command/DirectXCommandManager.h"
#include "SwapChain/SwapChain.h"
#include "Fence/Fence.h"

#ifdef _DEBUG
#include "AppUtility/DebugTool/DirectX/DirectX12DebugCore.h"
#endif // _DEBUG

// TODO: DepthStencilをここに置かない

class DirectXCommon final : public Singleton<DirectXCommon> {
	friend class Singleton<DirectXCommon>;

public:
	void Initialize(HWND hwnd, uint32_t width, uint32_t height);
	void PreDraw();
	void PostDraw();
	void Shutdown();

	[[nodiscard]] DescriptorHandles AssignRtvHeap(ID3D12Resource* resource, const D3D12_RENDER_TARGET_VIEW_DESC* desc);
	[[nodiscard]] DescriptorHandles AssignSrvHeap(ID3D12Resource* resource, const D3D12_SHADER_RESOURCE_VIEW_DESC& desc);
	[[nodiscard]] DescriptorHandles AssignDsvHeap(ID3D12Resource* resource, const D3D12_DEPTH_STENCIL_VIEW_DESC* desc);

	ID3D12Device* GetDevice();
	ID3D12GraphicsCommandList* GetCommandManager(const D3D12_COMMAND_LIST_TYPE& type);
	SwapChain* GetSwapChain();
	uint32_t GetBackBufferCount();
	Fence* GetFence();
	D3D12_RENDER_TARGET_VIEW_DESC& GetSwapChainRtvDesc();
	DescriptorHeapManager* GetDescriptorHeapManager();
	ID3D12DescriptorHeap* GetRtvDescriptorHeapAddress();
	ID3D12DescriptorHeap* GetSrvDescriptorHeapAddress();
	ID3D12DescriptorHeap* GetDsvDescriptorHeapAddress();
	ID3D12DescriptorHeap* const* GetRtvDescriptorHeapAddressOf();
	ID3D12DescriptorHeap* const* GetSrvDescriptorHeapAddressOf();
	ID3D12DescriptorHeap* const* GetDsvDescriptorHeapAddressOf();
	D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentBackBufferCpuHandle();

	DescriptorHandles* GetDepthStencilViewHandle();
	D3D12_VIEWPORT* GetViewPort();
	D3D12_RECT* GetScissorRect();

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
	D3D12_VIEWPORT viewport_;
	D3D12_RECT scissorRect_;

	Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilBuffer_;
	DescriptorHandles dsvHandle_;

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc_;
};