#include "SwapChain.h"
#include <cassert>

#include "EngineDefines.h"

using namespace QFE::GRAPHIC;

void SwapChain::CreateSwapChain(HWND hwnd, uint32_t width, uint32_t height, IDXGIFactory7* dxgiFactory, ID3D12CommandQueue* commandQueue) {
	// * SwapChain郢ｧ蝣､蜃ｽ隰瑚・笘・ｹｧ繝ｻ* //
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc_ = {};
	swapChainDesc_.Width = width;
	swapChainDesc_.Height = height;
	swapChainDesc_.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc_.SampleDesc.Count = 1;
	swapChainDesc_.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc_.BufferCount = 2;
	swapChainDesc_.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	// 郢ｧ・ｳ郢晄ｧｭﾎｦ郢晏ｳｨ縺冗ｹ晢ｽ･郢晢ｽｼ邵ｲ竏壹∴郢ｧ・｣郢晢ｽｳ郢晏ｳｨ縺育ｹ昜ｸ莞ｦ郢晏ｳｨﾎ晉ｸｲ竏ｬ・ｨ・ｭ陞ｳ螢ｹ・定ｲゑｽ｡邵ｺ蜉ｱ窶ｻ騾墓ｻ薙・
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
		QFE_LOG(std::format("Assign BackBuffer: {}", i));
	}
}

void SwapChain::Initialize(HWND hwnd, uint32_t width, uint32_t height, IDXGIFactory7* dxgiFactory, ID3D12CommandQueue* commandQueue) {
	CreateDubleBuffering();
	CreateSwapChain(hwnd, width, height, dxgiFactory, commandQueue);
	AssignBackbuffer();
}

ID3D12Resource* SwapChain::GetCurrentBackBuffer() const {
	return backBuffers_[swapChain_->GetCurrentBackBufferIndex()].Get();
}

D3D12_CPU_DESCRIPTOR_HANDLE SwapChain::GetCurrentBackBufferView() const {
	return backBufferViews_[swapChain_->GetCurrentBackBufferIndex()].cpuHandle_;
}

const D3D12_CPU_DESCRIPTOR_HANDLE* SwapChain::GetCurrentBackBufferViewPtr() const {
	return &backBufferViews_[swapChain_->GetCurrentBackBufferIndex()].cpuHandle_;
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

void SwapChain::AssignDescriptorHandles(const D3D12_CPU_DESCRIPTOR_HANDLE& rtvHandle, uint32_t index) {
#ifdef QFE_OPTIMIZE_OFF
	DebugLog(std::format("Add DescriptorHandles to SwapChain: {}", index));
#endif // QFE_OPTIMIZE_OFF
	assert(index < backBuffers_.size() && "Index out of range in AssignDescriptorHandles.");

	DescriptorHandles handles;
	handles.cpuHandle_ = rtvHandle;
	backBufferViews_[index] = handles;
}

bool SwapChain::CheckBackBufferViews() const {
#ifdef QFE_OPTIMIZE_OFF
	DebugLog(std::format("Buffer: {} View: {}", backBuffers_.size(), backBufferViews_.size()));
#endif // QFE_OPTIMIZE_OFF
	// 郢晁・繝｣郢ｧ・ｯ郢晁・繝｣郢晁ｼ斐＜邵ｺ邉ｻullptr邵ｺ・ｧ邵ｺ・ｪ邵ｺ繝ｻﾂｰ驕抵ｽｺ髫ｱ繝ｻ
	for (const auto& buffer : backBuffers_) {
		if (buffer == nullptr) {
			return false;
		}
	}
	// 郢晁・繝｣郢ｧ・ｯ郢晁・繝｣郢晁ｼ斐＜邵ｺ・ｮ隰ｨ・ｰ邵ｺ・ｨ郢晁侭ﾎ礼ｹ晢ｽｼ邵ｺ・ｮ隰ｨ・ｰ邵ｺ蠕｡・ｸﾂ髢ｾ・ｴ邵ｺ蜉ｱ窶ｻ邵ｺ繝ｻ・狗ｸｺ迢暦ｽ｢・ｺ髫ｱ繝ｻ
	if (backBufferViews_.size() != backBuffers_.size()) {
		return false;
	}
	return true;
}


