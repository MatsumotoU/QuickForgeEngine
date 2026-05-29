#include "D3D12GraphicEngine.h"

#include "dx12/DirectXDevice.h"
#include "dx12/DirectXResourceContainer.h"
#include "dx12/descriptors/DescriptorHeapManager.h"
#include "dx12/command/DirectXCommandManager.h"
#include "dx12/SwapChain.h"
#include "dx12/Fence.h"

QFE::GRAPHIC::D3D12GraphicEngine::D3D12GraphicEngine(HWND hwnd0):
	hwnd_(hwnd0),
	directXCommon_(std::make_unique<INTERNAL::DirectXDevice>()),
	resourceContainer_(std::make_unique<INTERNAL::DirectXResourceContainer>()),
	descriptorHeapManager_(std::make_unique<INTERNAL::DescriptorHeapManager>()),
	commandManager_(std::make_unique<INTERNAL::DirectXCommandManager>()),
	swapChain_(std::make_unique<INTERNAL::SwapChain>()),
	fence_(std::make_unique<INTERNAL::Fence>()) {}

void QFE::GRAPHIC::D3D12GraphicEngine::Initialize() {
	// DirectXDeviceの初期化
	directXCommon_->Initialize();
	// DescriptorHeapManagerの初期化
	descriptorHeapManager_->Initialize(directXCommon_->GetDevice());
	// CommandManagerの初期化
	commandManager_->Initialize(directXCommon_->GetDevice(), descriptorHeapManager_.get());
	// SwapChainの初期化
	swapChain_->Initialize(hwnd_, 1280, 720, directXCommon_->GetDxgiFactory(), commandManager_->GetCommandQueue());
	// Fenceの初期化
	fence_->Initialize(directXCommon_->GetDevice());
}

void QFE::GRAPHIC::D3D12GraphicEngine::PreDraw() {
}

void QFE::GRAPHIC::D3D12GraphicEngine::PostDraw() {
}

void QFE::GRAPHIC::D3D12GraphicEngine::Shutdown() {
}
