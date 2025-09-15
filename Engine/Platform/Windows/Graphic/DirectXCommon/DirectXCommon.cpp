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

	// depthStencilBufferの生成
	D3D12_RESOURCE_DESC depthResourceDesc{};
	depthResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthResourceDesc.Alignment = 0;
	depthResourceDesc.Width = width;
	depthResourceDesc.Height = height;
	depthResourceDesc.DepthOrArraySize = 1;
	depthResourceDesc.MipLevels = 1;
	depthResourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthResourceDesc.SampleDesc.Count = 1;
	depthResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	D3D12_HEAP_PROPERTIES depthHeapProps{};
	depthHeapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
	D3D12_CLEAR_VALUE depthClearValue{};
	depthClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthClearValue.DepthStencil.Depth = 1.0f;
	HRESULT hr = directXDevice_.GetDevice()->CreateCommittedResource(
		&depthHeapProps,
		D3D12_HEAP_FLAG_NONE,
		&depthResourceDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&depthClearValue,
		IID_PPV_ARGS(depthStencilBuffer_.GetAddressOf()));
	assert(SUCCEEDED(hr));
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	dsvHandle_ = descriptorHeapManager_.AssignDsvHeap(depthStencilBuffer_.Get(), &dsvDesc);

	// ビューポートとシザー矩形の設定
	viewport_ = {};
	viewport_.TopLeftX = 0.0f;
	viewport_.TopLeftY = 0.0f;
	viewport_.Width = static_cast<float>(width);
	viewport_.Height = static_cast<float>(height);
	viewport_.MinDepth = 0.0f;
	viewport_.MaxDepth = 1.0f;
	scissorRect_ = {};
	scissorRect_.left = 0;
	scissorRect_.top = 0;
	scissorRect_.right = static_cast<LONG>(width);
	scissorRect_.bottom = static_cast<LONG>(height);
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

DescriptorHandles DirectXCommon::AssignRtvHeap(ID3D12Resource* resource, const D3D12_RENDER_TARGET_VIEW_DESC* desc) {
	return descriptorHeapManager_.AssignRtvHeap(resource, desc);
}

DescriptorHandles DirectXCommon::AssignSrvHeap(ID3D12Resource* resource, const D3D12_SHADER_RESOURCE_VIEW_DESC& desc) {
	return descriptorHeapManager_.AssignSrvHeap(resource, desc);
}

DescriptorHandles DirectXCommon::AssignDsvHeap(ID3D12Resource* resource, const D3D12_DEPTH_STENCIL_VIEW_DESC* desc) {
	return descriptorHeapManager_.AssignDsvHeap(resource, desc);
}

void DirectXCommon::AssignSwapChainRenderTarget() {
	// スワップチェインのリソース登録
	rtvDesc_ = {};
	rtvDesc_.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	rtvDesc_.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	for (uint32_t i = 0; i < swapChain_.GetBackBufferCount(); ++i) {
		DescriptorHandles handles;
		handles = descriptorHeapManager_.AssignRtvHeap(swapChain_.GetBackBuffer(i), &rtvDesc_);
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

uint32_t DirectXCommon::GetBackBufferCount() {
	assert(swapChain_.GetBackBufferCount() > 0 && "BackBufferCount is 0.");
	return swapChain_.GetBackBufferCount();
}

Fence* DirectXCommon::GetFence() {
	assert(&fence_ && "Fence is nullptr.");
	return &fence_;
}

D3D12_RENDER_TARGET_VIEW_DESC& DirectXCommon::GetSwapChainRtvDesc() {
	assert(rtvDesc_.Format != DXGI_FORMAT_UNKNOWN && "RtvDesc is not set.");
	return rtvDesc_;
}

DescriptorHeapManager* DirectXCommon::GetDescriptorHeapManager() {
	return &descriptorHeapManager_;
}

ID3D12DescriptorHeap* DirectXCommon::GetRtvDescriptorHeapAddress() {
	return descriptorHeapManager_.GetRtvDescriptorHeapAddress();
}

ID3D12DescriptorHeap* DirectXCommon::GetSrvDescriptorHeapAddress() {
	return descriptorHeapManager_.GetSrvDescriptorHeapAddress();
}

ID3D12DescriptorHeap* DirectXCommon::GetDsvDescriptorHeapAddress() {
	return descriptorHeapManager_.GetDsvDescriptorHeapAddress();
}

ID3D12DescriptorHeap* const* DirectXCommon::GetRtvDescriptorHeapAddressOf() {
	return descriptorHeapManager_.GetRtvDescriptorHeapAddressOf();
}

ID3D12DescriptorHeap* const* DirectXCommon::GetSrvDescriptorHeapAddressOf() {
	return descriptorHeapManager_.GetSrvDescriptorHeapAddressOf();
}

ID3D12DescriptorHeap* const* DirectXCommon::GetDsvDescriptorHeapAddressOf() {
	return descriptorHeapManager_.GetDsvDescriptorHeapAddressOf();
}

D3D12_CPU_DESCRIPTOR_HANDLE DirectXCommon::GetCurrentBackBufferCpuHandle() {
	return swapChain_.GetCurrentBackBufferView();
}

DescriptorHandles* DirectXCommon::GetDepthStencilViewHandle() {
	return &dsvHandle_;
}

D3D12_VIEWPORT* DirectXCommon::GetViewPort() {
	return &viewport_;
}

D3D12_RECT* DirectXCommon::GetScissorRect() {
	return &scissorRect_;
}
