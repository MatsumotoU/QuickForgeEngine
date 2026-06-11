#include "SwapChain.h"
#include <cassert>

#include "EngineDefines.h"

namespace QFE::GRAPHIC::INTERNAL {

	void SwapChain::CreateSwapChain(HWND hwnd, uint32_t width, uint32_t height, IDXGIFactory7* dxgiFactory, ID3D12CommandQueue* commandQueue) {
		// * SwapChain生成関数* //
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc_ = {};
		swapChainDesc_.Width = width;
		swapChainDesc_.Height = height;
		swapChainDesc_.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc_.SampleDesc.Count = 1;
		swapChainDesc_.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc_.BufferCount = 2;
		swapChainDesc_.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		// 画面サイズ変更時、画面スケールに合わせる。
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
#ifdef QFE_OPTIMIZE_OFF
			QFE_LOG(std::format("Assign BackBuffer: {}", i));
#endif // QFE_OPTIMIZE_OFF
		}
	}

	void SwapChain::Initialize(HWND hwnd, uint32_t width, uint32_t height, IDXGIFactory7* dxgiFactory, ID3D12CommandQueue* commandQueue) {
		CreateDubleBuffering();
		CreateSwapChain(hwnd, width, height, dxgiFactory, commandQueue);
		AssignBackbuffer();
	}

	void SwapChain::ClearBackBuffer(ID3D12GraphicsCommandList* commandList, const FLOAT clearColor[4]) {
		assert(commandList != nullptr && "CommandList is nullptr in ClearBackBuffer.");
		assert(CheckBackBufferViews() && "BackBufferViews are not properly assigned in ClearBackBuffer.");
		for (uint32_t i = 0; i < backBuffers_.size(); ++i) {
			commandList->ClearRenderTargetView(backBufferViews_[i].cpuHandle_, clearColor, 0, nullptr);
			QFE_LOG(std::format("Clear BackBuffer: {}", i));
		}
	}

	void SwapChain::TransitionCurrentBackBufferToRenderTarget(ID3D12GraphicsCommandList* commandList) {
		assert(commandList != nullptr && "CommandList is nullptr in TransitionCurrentBackBufferToRenderTarget.");
		assert(CheckBackBufferViews() && "BackBufferViews are not properly assigned in TransitionCurrentBackBufferToRenderTarget.");
		
		D3D12_RESOURCE_BARRIER barrier = {};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Transition.pResource = GetCurrentBackBuffer();
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		commandList->ResourceBarrier(1, &barrier);
	}

	void SwapChain::TransitionCurrentBackBufferToPresent(ID3D12GraphicsCommandList* commandList) {
		assert(commandList != nullptr && "CommandList is nullptr in TransitionCurrentBackBufferToPresent.");
		assert(CheckBackBufferViews() && "BackBufferViews are not properly assigned in TransitionCurrentBackBufferToPresent.");

		D3D12_RESOURCE_BARRIER barrier = {};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Transition.pResource = GetCurrentBackBuffer();
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		commandList->ResourceBarrier(1, &barrier);
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
		backBuffers_.push_back(nullptr);
		backBuffers_.push_back(nullptr);
		backBufferViews_.push_back({});
		backBufferViews_.push_back({});
	}

	void SwapChain::Present() {
		HRESULT hr = swapChain_->Present(1, 0);
		hr;
		assert(SUCCEEDED(hr) && "Failed to present the swap chain.");
	}

	void SwapChain::AssignDescriptorHandles(const DescriptorHandles& rtvHandle, uint32_t index) {
		QFE_LOG(std::format("Add DescriptorHandles to SwapChain: {}", index));

		assert(index < backBuffers_.size() && "Index out of range in AssignDescriptorHandles.");
		backBufferViews_[index] = rtvHandle;
	}

	bool SwapChain::CheckBackBufferViews() const {
		// backBuffers_とbackBufferViews_のサイズをログに出力
		QFE_LOG(std::format("SwapChain BufferSize: {} ViewSize: {}", backBuffers_.size(), backBufferViews_.size()));
		// バックバッファがnullptrではないか確認
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

}
