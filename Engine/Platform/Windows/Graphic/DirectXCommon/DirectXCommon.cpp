#include "DirectXCommon.h"
#include <cassert>

#include "AppUtility/DirectX/TransitionResourceBarrier.h"
#include "AppUtility/DirectX/InitializeSwapChainBuffer.h"

namespace {
	float clearColor_[4] = { 0.1f, 0.25f, 0.5f, 1.0f };
}

void DirectXCommon::Initialize(HWND hwnd, uint32_t width, uint32_t height) {
	directXDevice_.Initialize();
	descriptorHeapManager_.Initialize(directXDevice_.GetDevice());
	
	commandManager_.Initialize(directXDevice_.GetDevice());

	swapChain_.CreateDubleBuffering();
	swapChain_.Initialize(
		hwnd, width, height, directXDevice_.GetDxgiFactory(), commandManager_.GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT));
	
	AssignSwapChainRenderTarget();

	fence_.Initialize(directXDevice_.GetDevice());
}

void DirectXCommon::PreDraw() {
	// スワップチェインのリソース状態を描画可能に変更
	TransitionResourceBarrier::Transition(
		commandManager_.GetCommandList(D3D12_COMMAND_LIST_TYPE_DIRECT),
		swapChain_.GetCurrentBackBuffer(),
		D3D12_RESOURCE_STATE_PRESENT,
		D3D12_RESOURCE_STATE_RENDER_TARGET);

	InitializeSwapChainBuffer::Execute(
		commandManager_.GetCommandList(D3D12_COMMAND_LIST_TYPE_DIRECT), swapChain_.GetCurrentBackBufferView(),
		clearColor_[0], clearColor_[1], clearColor_[2], clearColor_[3]);
}

void DirectXCommon::PostDraw() {
	// スワップチェインのリソース状態を読み取りに変更
	TransitionResourceBarrier::Transition(
		commandManager_.GetCommandList(D3D12_COMMAND_LIST_TYPE_DIRECT),
		swapChain_.GetCurrentBackBuffer(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PRESENT);
	
	// コマンドリストのクローズ、実行
	commandManager_.ExecuteCommandList();

	// スワップチェインの画面への表示
	swapChain_.Present();

	// GPUとの同期
	fence_.Signal(commandManager_.GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT));
	fence_.Wait();

	// コマンドリストのリセット
	commandManager_.ResetCommandList();
}

void DirectXCommon::Shutdown() {
	fence_.Shutdown();
}

void DirectXCommon::AssignSwapChainRenderTarget() {
	// スワップチェインのリソース登録
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	for (uint32_t i = 0; i < swapChain_.GetBackBufferCount(); ++i) {
		DescriptorHandles handles;
		handles = descriptorHeapManager_.AssignRtvHeap(swapChain_.GetBackBuffer(i), &rtvDesc);
		swapChain_.AssignDescriptorHandles(handles, i);
	}
	assert(swapChain_.CheckBackBufferViews());
}

ID3D12Device* DirectXCommon::GetDevice() {
	assert(directXDevice_.GetDevice() && "DirectXDevice is nullptr.");
	return directXDevice_.GetDevice();
}

ID3D12GraphicsCommandList* DirectXCommon::GetCommandManager(const D3D12_COMMAND_LIST_TYPE& type) {
	assert(commandManager_.GetCommandList(type) && "CommandManager is nullptr.");
	return commandManager_.GetCommandList(type);
}

SwapChain* DirectXCommon::GetSwapChain() {
	assert(&swapChain_ && "SwapChain is nullptr.");
	return &swapChain_;
}

Fence* DirectXCommon::GetFence() {
	assert(&fence_ && "Fence is nullptr.");
	return &fence_;
}

ID3D12DescriptorHeap* DirectXCommon::GetRtvDescriptorHeapAddress() {
	return descriptorHeapManager_.GetRtvDescriptorHeapAddress();
}

ID3D12DescriptorHeap* DirectXCommon::GetSrvDescriptorHeapAddress() {
	return descriptorHeapManager_.GetSrvDescriptorHeapAddress();
}

ID3D12DescriptorHeap* const* DirectXCommon::GetRtvDescriptorHeapAddressOf() {
	return descriptorHeapManager_.GetRtvDescriptorHeapAddressOf();
}

ID3D12DescriptorHeap* const* DirectXCommon::GetSrvDescriptorHeapAddressOf() {
	return descriptorHeapManager_.GetSrvDescriptorHeapAddressOf();
}
