#include "D3D12GraphicEngine.h"
#include "EngineDefines.h"

#include "dx12/checker/DirectX12DebugCore.h"
#include "dx12/DirectXDevice.h"
#include "dx12/DirectXResourceContainer.h"
#include "dx12/descriptors/DescriptorHeapManager.h"
#include "dx12/command/DirectXCommandManager.h"
#include "dx12/SwapChain.h"
#include "dx12/Fence.h"

#include "dx12/pipeline/GraphicPipelineManager.h"
#include "dx12/TextureManager.h"
#include "dx12/ModelVertexResourceManager.h"

#include "dx12/descriptors/Data/DescriptorHandles.h"

namespace {
	float clearColor[4] = { 0.1f, 0.25f, 0.5f, 1.0f };
}

QFE::GRAPHIC::D3D12GraphicEngine::D3D12GraphicEngine(HWND hwnd0) :
	hwnd_(hwnd0),
	debugCore_(std::make_unique<INTERNAL::DirectX12DebugCore>()),
	directXDevice_(std::make_unique<INTERNAL::DirectXDevice>()),
	resourceContainer_(std::make_unique<INTERNAL::DirectXResourceContainer>()),
	descriptorHeapManager_(std::make_unique<INTERNAL::DescriptorHeapManager>()),
	commandManager_(std::make_unique<INTERNAL::DirectXCommandManager>()),
	swapChain_(std::make_unique<INTERNAL::SwapChain>()),
	fence_(std::make_unique<INTERNAL::Fence>()),
	graphicPipelineManager_(std::make_unique<INTERNAL::GraphicPipelineManager>()),
	textureManager_(std::make_unique<INTERNAL::TextureManager>()),
	modelVertexResourceManager_(std::make_unique<INTERNAL::ModelVertexResourceManager>())

{}

QFE::GRAPHIC::D3D12GraphicEngine::~D3D12GraphicEngine() = default;

void QFE::GRAPHIC::D3D12GraphicEngine::Initialize() {
	// ウィンドウのクライアント領域のサイズを取得
	RECT rect;
	GetClientRect(hwnd_, &rect);
	uint32_t width = static_cast<uint32_t>(rect.right - rect.left);
	uint32_t height = static_cast<uint32_t>(rect.bottom - rect.top);

	// DirectXDeviceの初期化
	directXDevice_->Initialize();
	// DescriptorHeapManagerの初期化
	descriptorHeapManager_->Initialize(directXDevice_->GetDevice());
	// CommandManagerの初期化
	commandManager_->Initialize(directXDevice_->GetDevice());
	// SwapChainの初期化
	swapChain_->Initialize(
		hwnd_, width, height,
		directXDevice_->GetDxgiFactory(),
		commandManager_->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT));
	// Fenceの初期化
	fence_->Initialize(directXDevice_->GetDevice());

	// DirectXCommonの名残.フェンスの初期化以降の処理.
	LegacyInitialize(width, height);

	// グラフィックパイプラインの初期化
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc = {};
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	graphicPipelineManager_->Initialize(directXDevice_->GetDevice(), depthStencilDesc);

	// テクスチャ管理クラスの初期化
	textureManager_->Initialize(
		directXDevice_->GetDevice(), 
		commandManager_->GetCommandList(D3D12_COMMAND_LIST_TYPE_DIRECT), 
		descriptorHeapManager_->GetSrvDescriptorHeap());

	// モデル頂点リソース管理クラスの初期化
	modelVertexResourceManager_->Initialize();
}

void QFE::GRAPHIC::D3D12GraphicEngine::PreDraw() {
	// スワップチェーンのリソース状態を描画可能に変更
	swapChain_->TransitionCurrentBackBufferToRenderTarget(commandManager_->GetCommandList(D3D12_COMMAND_LIST_TYPE_DIRECT));

	{
		float color[] = { clearColor[0], clearColor[1], clearColor[2], clearColor[3] };
		commandManager_->GetCommandList(D3D12_COMMAND_LIST_TYPE_DIRECT)->ClearRenderTargetView(swapChain_->GetCurrentBackBufferView(), color, 0, nullptr);
	}

	// デプスステンシルのクリア
	ClearDepthStencil();
}

void QFE::GRAPHIC::D3D12GraphicEngine::PostDraw() {
	// スワップチェーンのリソース状態を表示可能に変更
	swapChain_->TransitionCurrentBackBufferToPresent(commandManager_->GetCommandList(D3D12_COMMAND_LIST_TYPE_DIRECT));

	// コマンドリストをクローズ、実行
	commandManager_->ExecuteCommandList();

	// スワップチェーンの画面への表示
	swapChain_->Present();

	// GPUとの同期
	fence_->Signal(commandManager_->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT));
	fence_->Wait();

	// コマンドリストをリセット
	commandManager_->ResetCommandList();

	// テクスチャのアップロードに使用した中間リソースの解放
	textureManager_->ReleaseIntermediateResources();
}

void QFE::GRAPHIC::D3D12GraphicEngine::Shutdown() {
	modelVertexResourceManager_->Finalize();
	textureManager_->Finalize();
	fence_->Shutdown();
	directXDevice_->Shutdown();
}

void QFE::GRAPHIC::D3D12GraphicEngine::LegacyInitialize(uint32_t width, uint32_t height) {
	AssignSwapChainDescriptor();
	CreateDepthStencilBuffer(width, height);
	CreateViewportAndScissorRect(width, height);
}

void QFE::GRAPHIC::D3D12GraphicEngine::AssignSwapChainDescriptor() {
	// スワップチェーンのリソース登録
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc_ = {};
	rtvDesc_.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	rtvDesc_.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	for (uint32_t i = 0; i < swapChain_->GetBackBufferCount(); ++i) {
		INTERNAL::DescriptorHandles handles;
		handles = descriptorHeapManager_->AssignRtvHeap(swapChain_->GetBackBuffer(i), &rtvDesc_);
		swapChain_->AssignDescriptorHandles(handles, i);
	}
	assert(swapChain_->CheckBackBufferViews());
}

void QFE::GRAPHIC::D3D12GraphicEngine::CreateDepthStencilBuffer(uint32_t width, uint32_t height) {
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
	HRESULT hr = directXDevice_->GetDevice()->CreateCommittedResource(
		&depthHeapProps,
		D3D12_HEAP_FLAG_NONE,
		&depthResourceDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&depthClearValue,
		IID_PPV_ARGS(depthStencilBuffer_.GetAddressOf()));
	assert(SUCCEEDED(hr));
	hr;
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	dsvHandle_ = descriptorHeapManager_->AssignDsvHeap(depthStencilBuffer_.Get(), &dsvDesc);
}

void QFE::GRAPHIC::D3D12GraphicEngine::CreateViewportAndScissorRect(uint32_t width, uint32_t height) {
	viewport_.TopLeftX = 0.0f;
	viewport_.TopLeftY = 0.0f;
	viewport_.Width = static_cast<float>(width);
	viewport_.Height = static_cast<float>(height);
	viewport_.MinDepth = 0.0f;
	viewport_.MaxDepth = 1.0f;
	scissorRect_.left = 0;
	scissorRect_.top = 0;
	scissorRect_.right = static_cast<LONG>(width);
	scissorRect_.bottom = static_cast<LONG>(height);
}

void QFE::GRAPHIC::D3D12GraphicEngine::ClearDepthStencil() {
	commandManager_->GetCommandList(D3D12_COMMAND_LIST_TYPE_DIRECT)->ClearDepthStencilView(
		dsvHandle_.cpuHandle_, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
}
