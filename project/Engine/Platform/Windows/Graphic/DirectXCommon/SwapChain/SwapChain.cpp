#include "SwapChain.h"
#include <cassert>

#ifdef _DEBUG
#include "AppUtility/DebugTool/DebugLog/MyDebugLog.h"
#endif // _DEBUG

void SwapChain::CreateSwapChain(HWND hwnd, uint32_t width, uint32_t height, IDXGIFactory7* dxgiFactory, ID3D12CommandQueue* commandQueue) {
	// * SwapChainを生成する * //
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc_ = {};
	swapChainDesc_.Width = width;
	swapChainDesc_.Height = height;
	swapChainDesc_.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc_.SampleDesc.Count = 1;
	swapChainDesc_.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc_.BufferCount = 2;
	swapChainDesc_.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	// コマンドキュー、ウィンドウハンドル、設定を渡して生成
	HRESULT hr = dxgiFactory->CreateSwapChainForHwnd(commandQueue, hwnd, &swapChainDesc_, nullptr, nullptr, reinterpret_cast<IDXGISwapChain1**>(swapChain_.GetAddressOf()));
	hr;
	assert(SUCCEEDED(hr));
}

void SwapChain::AssignBackbuffer() {
	assert(swapChain_.Get() != nullptr && "SwapChain is nullptr");
	assert(!backBuffers_.empty() && "BackBuffer is not empty");

	HRESULT hr;
	for (int i = 0; i < backBuffers_.size(); ++i) {
		hr = swapChain_.Get()->GetBuffer(i, IID_PPV_ARGS(backBuffers_.at(i).GetAddressOf()));
		assert(SUCCEEDED(hr));
#ifdef _DEBUG
		DebugLog(std::format("Assign BackBuffer: {}", i));
#endif // _DEBUG
	}
}

void SwapChain::Initialize(HWND hwnd, uint32_t width, uint32_t height, IDXGIFactory7* dxgiFactory, ID3D12CommandQueue* commandQueue) {
	CreateSwapChain(hwnd, width, height, dxgiFactory, commandQueue);
	AssignBackbuffer();
}

ID3D12Resource* SwapChain::GetCurrentBackBuffer() const {
	return backBuffers_[swapChain_->GetCurrentBackBufferIndex()].Get();
}

D3D12_CPU_DESCRIPTOR_HANDLE SwapChain::GetCurrentBackBufferView() const {
	return backBufferViews_[swapChain_->GetCurrentBackBufferIndex()].cpuHandle_;
}

uint32_t SwapChain::GetCurrentBackBufferIndex() const {
	return swapChain_->GetCurrentBackBufferIndex();
}

ID3D12Resource* SwapChain::GetBackBuffer(uint32_t index) const {
	assert(index < backBuffers_.size() && "Index out of range in GetBackBuffer.");
	assert(backBuffers_[index] != nullptr && "BackBuffer is nullptr in GetBackBuffer.");
	return backBuffers_[index].Get();
}

uint32_t SwapChain::GetBackBufferCount() const {
	return static_cast<uint32_t>(backBuffers_.size());
}

void SwapChain::CreateDubleBuffering() {
	backBuffers_.resize(2);
	backBuffers_[0] = nullptr;
	backBuffers_[1] = nullptr;
	backBufferViews_.resize(2);
	backBufferViews_[0] = {};
	backBufferViews_[1] = {};
}

void SwapChain::Present() {
	HRESULT hr = swapChain_->Present(1, 0);
	hr;
	assert(SUCCEEDED(hr) && "Failed to present the swap chain.");
}

void SwapChain::AssignDescriptorHandles(const DescriptorHandles& rtvHandle, uint32_t index) {
#ifdef _DEBUG
	DebugLog(std::format("Add DescriptorHandles to SwapChain: {}", index));
#endif // _DEBUG
	assert(index < backBuffers_.size() && "Index out of range in AssignDescriptorHandles.");
	backBufferViews_[index] = rtvHandle;
}

bool SwapChain::CheckBackBufferViews() const {
#ifdef _DEBUG
	DebugLog(std::format("Buffer: {} View: {}", backBuffers_.size(), backBufferViews_.size()));
#endif // _DEBUG
	// バックバッファがnullptrでないか確認
	for (const auto& buffer : backBuffers_) {
		if (buffer == nullptr) {
			return false;
		}
	}
	// バックバッファの数とビューの数が一致しているか確認
	if (backBufferViews_.size() != backBuffers_.size()) {
		return false;
	}
	return true;
}
