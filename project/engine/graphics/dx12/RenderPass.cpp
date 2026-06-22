#include "RenderPass.h"
#include "SwapChain.h"

#include "EngineDefines.h"

using namespace QFE::GRAPHIC;

namespace {
	const float kClearColor[4] = { 0.1f, 0.25f, 0.5f, 1.0f };
}

QFE::GRAPHIC::RenderPass::RenderPass() = default;
QFE::GRAPHIC::RenderPass::~RenderPass() = default;

void RenderPass::Initialize(const RenderPassInitializeInfo& initializeInfo) {
	// 引数の検査
	if (!initializeInfo.device) {
		QFE_REPORT_SYSTEM_ERROR("Device is null in RenderPass::Initialize", SystemError::Abort);
		return;
	}
	if (!initializeInfo.dxgiFactory) {
		QFE_REPORT_SYSTEM_ERROR("DXGI Factory is null in RenderPass::Initialize", SystemError::Abort);
		return;
	}
	if (!initializeInfo.commandQueue) {
		QFE_REPORT_SYSTEM_ERROR("Command Queue is null in RenderPass::Initialize", SystemError::Abort);
		return;
	}
	QFE_LOG(std::format("Initializing RenderPass with width: {}, height: {}", initializeInfo.width, initializeInfo.height));

	if(!initializeInfo.getResourceDsvFunc) {
		QFE_REPORT_SYSTEM_ERROR("GetResourceDsvFunc is not set in RenderPass::Initialize", SystemError::Abort);
		return;
	}

	// SwapChainの初期化
	swapChain_ = std::make_unique<SwapChain>();
	swapChain_->Initialize(
		initializeInfo.hwnd,initializeInfo.width,initializeInfo.height,
		initializeInfo.dxgiFactory,initializeInfo.commandQueue);
	// レンダービューに登録
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	for (uint32_t i = 0; i < swapChain_->GetBackBufferCount(); ++i) {
		D3D12_CPU_DESCRIPTOR_HANDLE handle = initializeInfo.assginRtvFunc(swapChain_->GetBackBuffer(i),&rtvDesc);
		swapChain_->AssignDescriptorHandles(handle, i);
	}

	// RenderPassの初期化情報を保存
	initializeInfo_ = initializeInfo;
}

void RenderPass::PreDraw(ID3D12GraphicsCommandList* commandList) {
	// スワップチェーンのバックバッファを描画用に変更するバリアを発行
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = swapChain_->GetCurrentBackBuffer();
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	commandList->ResourceBarrier(1, &barrier);
	
	// バックバッファをクリア
	commandList->ClearRenderTargetView(swapChain_->GetCurrentBackBufferView(), kClearColor, 0, nullptr);
}

void RenderPass::PostDraw(ID3D12GraphicsCommandList* commandList) {
	// スワップチェーンのバックバッファを読み込み用に変更するバリアを発行
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = swapChain_->GetCurrentBackBuffer();
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	commandList->ResourceBarrier(1, &barrier);
}

void RenderPass::Present() {
	swapChain_->Present();
}

void QFE::GRAPHIC::RenderPass::SetRenderTarget(
	ID3D12GraphicsCommandList* commandList, DirectXResourceHandle depthStencilHandle, RenderTargetHandle renderTargetHandle) {

	if (renderTargetHandle == RenderTargetHandle::SwapChain) {
		commandList->OMSetRenderTargets(1, swapChain_->GetCurrentBackBufferViewPtr(), FALSE, initializeInfo_.getResourceDsvFunc(depthStencilHandle));
	} else {
		// オフスクリーンは未実装
		QFE_REPORT_SYSTEM_ERROR("Offscreen render target is not implemented yet in RenderPass::SetRenderTarget", SystemError::Abort);
	}
}

void RenderPass::TransitionCurrentBackBufferBarrier(ID3D12GraphicsCommandList* commandList, D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState) {
		D3D12_RESOURCE_BARRIER barrier{};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource = swapChain_->GetCurrentBackBuffer();
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		barrier.Transition.StateBefore = beforeState;
		barrier.Transition.StateAfter = afterState;
		commandList->ResourceBarrier(1, &barrier);
}

ID3D12Resource* RenderPass::GetCurrentBackBuffer() const {
	return swapChain_->GetCurrentBackBuffer();
}