#include "RenderPass.h"

#include "SwapChain.h"
#include "OffscreenBuffer.h"

#include "EngineDefines.h"

using namespace QFE::GRAPHIC::INTERNAL;

namespace {
	const float kClearColor[4] = { 0.1f, 0.25f, 0.5f, 1.0f };
	const uint32_t kOffscreenCount = 16;
}

QFE::GRAPHIC::INTERNAL::RenderPass::RenderPass() = default;
QFE::GRAPHIC::INTERNAL::RenderPass::~RenderPass() = default;

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

	// SwapChainの初期化
	swapChain_ = std::make_unique<SwapChain>();
	swapChain_->Initialize(
		initializeInfo.hwnd, initializeInfo.width, initializeInfo.height,
		initializeInfo.dxgiFactory, initializeInfo.commandQueue);
	// バックバッファをRTVに割り当て
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc_ = {};
	rtvDesc_.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	rtvDesc_.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	for (uint32_t i = 0; i < swapChain_->GetBackBufferCount(); ++i) {
		swapChain_->AssignDescriptorHandles(
			initializeInfo.assignRtvFunc(swapChain_->GetBackBuffer(i), &rtvDesc_), i);
	}

	// OffscreenBufferの初期化
	offscreenBuffer_ = std::make_unique<OffscreenBuffer>();
	OffscreenBufferInitializeInfo offscreenInitializeInfo = {};
	offscreenInitializeInfo.device = initializeInfo.device;
	offscreenInitializeInfo.width = initializeInfo.width;
	offscreenInitializeInfo.height = initializeInfo.height;
	offscreenInitializeInfo.assignRtvFunc = initializeInfo.assignRtvFunc;
	offscreenInitializeInfo.assignSrvFunc = initializeInfo.assignSrvFunc;
	// kOffscreenCount個のオフスクリーンを生成し、ハンドルを保存
	for (uint32_t i = 0; i < kOffscreenCount; ++i) {
		offscreenHandles_.push_back(offscreenBuffer_->Create(offscreenInitializeInfo));
	}
}

void RenderPass::PreDraw(ID3D12GraphicsCommandList* commandList) {
	TransitionRenderTargetToRenderTarget(commandList);
	ClearRenderTarget(commandList, kClearColor);


}

void RenderPass::PostDraw(ID3D12GraphicsCommandList* commandList) {
	TransitionRenderTargetToPresent(commandList);
}

void RenderPass::Present() {
	swapChain_->Present();
}

void RenderPass::SetRenderTarget(ID3D12GraphicsCommandList* commandList, RenderTargetHandle renderTargetHandle) {
	if (renderTargetHandle == RenderTargetHandle::SwapChain) {
		// バックバッファを描画先に設定
		commandList->OMSetRenderTargets(1, swapChain_->GetCurrentBackBufferViewPtr(), FALSE, nullptr);
	} else {
		// オフスクリーンを描画先に設定
		uint32_t offscreenIndex = static_cast<uint32_t>(renderTargetHandle) - 1;
		if (offscreenIndex < offscreenHandles_.size()) {
			offscreenBuffer_->SetRenderTarget(commandList, offscreenHandles_[offscreenIndex]);
			commandList->OMSetRenderTargets(1, offscreenBuffer_->GetRtvHandlePtr(offscreenHandles_[offscreenIndex]), FALSE, nullptr);
		} else {
			QFE_REPORT_SYSTEM_ERROR(std::format("Invalid render target handle: {}", static_cast<uint32_t>(renderTargetHandle)), SystemError::Abort);
		}
	}
}

void RenderPass::TransitionRenderTargetToRenderTarget(ID3D12GraphicsCommandList* commandList) {
	// バックバッファを描画用に変更
	swapChain_->TransitionCurrentBackBufferToRenderTarget(commandList);
	// オフスクリーンを描画用に変更
	for (uint32_t handle : offscreenHandles_) {
		offscreenBuffer_->SetRenderTarget(commandList, handle);
	}
}

void RenderPass::TransitionRenderTargetToPresent(ID3D12GraphicsCommandList* commandList) {
	// バックバッファを読み込み用に変更
	swapChain_->TransitionCurrentBackBufferToPresent(commandList);
	// オフスクリーンを描画用に変更
	for (uint32_t handle : offscreenHandles_) {
		offscreenBuffer_->SetTexture(commandList, handle);
	}
}

void RenderPass::ClearRenderTarget(ID3D12GraphicsCommandList* commandList, const float clearColor[4]) {
	// バックバッファをクリア
	float color[] = { clearColor[0], clearColor[1], clearColor[2], clearColor[3] };
	commandList->ClearRenderTargetView(swapChain_->GetCurrentBackBufferView(), color, 0, nullptr);
	// オフスクリーンをクリア
	for (uint32_t handle : offscreenHandles_) {
		offscreenBuffer_->Clear(commandList, handle);
	}

}