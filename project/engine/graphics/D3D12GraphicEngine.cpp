#include "D3D12GraphicEngine.h"
#include "EngineDefines.h"

#include "dx12/checker/DirectX12DebugCore.h"

#include "dx12/vram/descriptors/DescriptorHeapManager.h"
#include "dx12/command/DirectXCommandManager.h"
#include "dx12/RenderPass.h"
#include "dx12/Fence.h"

#include "dx12/pipeline/pso/ShaderCompiler.h"
#include "dx12/pipeline/pso/ShaderReflection.h"
#include "dx12/pipeline/rtpso/ShaderLibReflection.h"

#include "dx12/pipeline/GraphicPipelineManager.h"
#include "dx12/pipeline/ComputePipelineManager.h"
#include "dx12/pipeline/RayTracingPipelineManager.h"

#include "dx12/TextureLoader.h"

#include "dx12/vram/descriptors/DescriptorHandles.h"

#include "string/MyString.h"

using namespace QFE::GRAPHIC;

namespace {
	float clearColor[4] = { 0.1f, 0.25f, 0.5f, 1.0f };
	const uint32_t kInvalidTextureHandle = UINT32_MAX;
}

QFE::GRAPHIC::D3D12GraphicEngine::D3D12GraphicEngine(HWND hwnd0) :
	hwnd_(hwnd0),
	debugCore_(std::make_unique<DirectX12DebugCore>()),
	directXDevice_(std::make_unique<DirectXDevice>()),
	resourceContainer_(std::make_unique<DirectXResourceContainer>()),
	descriptorHeapManager_(std::make_unique<DescriptorHeapManager>()),
	commandManager_(std::make_unique<DirectXCommandManager>()),
	renderPass_(std::make_unique<RenderPass>()),
	fence_(std::make_unique<Fence>()),
	graphicPipelineManager_(std::make_unique<GraphicPipelineManager>()),
	textureLoader_(std::make_unique<TextureLoader>()),
	shaderCompiler_(std::make_unique<ShaderCompiler>()),
	shaderReflection_(std::make_unique<ShaderReflection>()),
	computePipelineManager_(std::make_unique<ComputePipelineManager>()),
	rayTracingPipelineManager_(std::make_unique<RaytracingPipelineManager>()),
	shaderLibReflection_(std::make_unique<ShaderLibReflection>()),
	resourceAllocator_(std::make_unique<DirectXResourceAllocator>())

{
}

D3D12GraphicEngine::~D3D12GraphicEngine() = default;

void D3D12GraphicEngine::Initialize() {
	// ウィンドウのクライアント領域のサイズを取得
	RECT rect;
	GetClientRect(hwnd_, &rect);
	uint32_t width = static_cast<uint32_t>(rect.right - rect.left);
	uint32_t height = static_cast<uint32_t>(rect.bottom - rect.top);

	// DirectXDeviceの初期化
	directXDevice_->Initialize();
	// シェーダーコンパイルに必要なDXCデバイスの初期化
	shaderCompiler_->Initialize();
	// DescriptorHeapManagerの初期化
	descriptorHeapManager_->Initialize(directXDevice_->GetDevice());
	// CommandManagerの初期化
	commandManager_->Initialize(directXDevice_->GetDevice());

	// ResourceContainerの初期化
	DirectXResourceContainerInitializeInfo resourceContainerInfo{};
	resourceContainerInfo.assignRtvFunc = [&](ID3D12Resource* resource, const D3D12_RENDER_TARGET_VIEW_DESC* desc)
		{return descriptorHeapManager_->AssignRtvHeap(directXDevice_->GetDevice(), resource, desc); };
	resourceContainerInfo.assignSrvFunc = [&](ID3D12Resource* resource, const D3D12_SHADER_RESOURCE_VIEW_DESC* desc)
		{return descriptorHeapManager_->AssignSrvHeap(directXDevice_->GetDevice(), resource, *desc); };
	resourceContainerInfo.assignDsvFunc = [&](ID3D12Resource* resource, const D3D12_DEPTH_STENCIL_VIEW_DESC* desc)
		{return descriptorHeapManager_->AssignDsvHeap(directXDevice_->GetDevice(), resource, desc); };
	resourceContainerInfo.assignUavFunc = [&](ID3D12Resource* resource, const D3D12_UNORDERED_ACCESS_VIEW_DESC* desc)
		{return descriptorHeapManager_->AssignUavHeap(directXDevice_->GetDevice(), resource, nullptr, desc); };
	resourceContainerInfo.assignTextureFunc = [&](ID3D12Resource* resource, const D3D12_SHADER_RESOURCE_VIEW_DESC* desc)
		{return descriptorHeapManager_->AssignTexture(directXDevice_->GetDevice(), resource, *desc); };
	resourceContainer_->Initialize(resourceContainerInfo);

	// リソースアロケータの初期化
	resourceAllocator_->Initialize(resourceContainer_.get(), directXDevice_->GetDevice());

	// RenderPassの初期化設定
	RenderPassInitializeInfo renderPassInfo{};
	renderPassInfo.width = width;
	renderPassInfo.height = height;
	renderPassInfo.hwnd = hwnd_;
	renderPassInfo.device = directXDevice_->GetDevice();
	renderPassInfo.dxgiFactory = directXDevice_->GetDxgiFactory();
	renderPassInfo.commandQueue = commandManager_->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
	// リソースの状態を変更する関数
	renderPassInfo.transitionFunc = [&](DirectXResourceHandle resourceHandle, D3D12_RESOURCE_STATES newState)
		{return resourceContainer_->TransitionResource(resourceHandle, commandManager_->GetCommandList(D3D12_COMMAND_LIST_TYPE_DIRECT), newState); };
	// Rtvを取得する関数
	renderPassInfo.getResourceRtvFunc = [&](DirectXResourceHandle resourceHandle)
		{return resourceContainer_->GetDescriptorHandleCpuPtr(resourceHandle, ViewTypeFlags::RenderTargetView); };
	// Dsvを取得する関数
	renderPassInfo.getResourceDsvFunc = [&](DirectXResourceHandle resourceHandle)
		{return resourceContainer_->GetDescriptorHandleCpuPtr(resourceHandle, ViewTypeFlags::DepthStencilView); };
	// Rtvに割り当てる関数
	renderPassInfo.assginRtvFunc = [&](ID3D12Resource* resource, const D3D12_RENDER_TARGET_VIEW_DESC* desc)
		{return descriptorHeapManager_->AssignRtvHeap(directXDevice_->GetDevice(), resource, desc).cpuHandle_; };
	// オフスクリーンレンダーターゲットを作成する関数
	renderPassInfo.createOffscreenFunc = [&](uint32_t width, uint32_t height, DXGI_FORMAT format) {
		D3D12_RESOURCE_DESC resourceDesc = {};
		resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		resourceDesc.Format = format;
		resourceDesc.Width = width;
		resourceDesc.Height = height;
		resourceDesc.DepthOrArraySize = 1;
		resourceDesc.MipLevels = 1;
		resourceDesc.SampleDesc.Count = 1;
		resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
		// クリア値の設定
		D3D12_CLEAR_VALUE clearValue = {};
		clearValue.Format = format;
		clearValue.Color[0] = 0.0f;
		clearValue.Color[1] = 0.0f;
		clearValue.Color[2] = 0.0f;
		clearValue.Color[3] = 0.0f;
		// リソースを作成し、リソースハンドルを返す
		DirectXResourceHandle handle = resourceContainer_->CreateResource(
			directXDevice_->GetDevice(), resourceDesc,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_HEAP_TYPE_DEFAULT, &clearValue);
		// RTVを作成
		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
		rtvDesc.Format = format;
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		// SRVを作成
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = format;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Texture2D.MipLevels = 1;
		// ResourceViewを作成
		CreateViewInfo viewInfo{};
		viewInfo.viewType = ViewTypeFlags::RenderTargetView | ViewTypeFlags::ShaderResourceView;
		viewInfo.rtvDesc = rtvDesc;
		viewInfo.srvDesc = srvDesc;
		resourceContainer_->CreateResourceView(handle, viewInfo);
		return handle;
		};

	// RenderPassの初期化
	renderPass_->Initialize(renderPassInfo);

	// Fenceの初期化
	fence_->Initialize(directXDevice_->GetDevice());

	// DirectXCommonの名残.フェンスの初期化以降の処理.
	LegacyInitialize(width, height);

	// グラフィックパイプラインマネージャの初期化
	GraphicPipelineManagerInitializeInfo graphicPipelineManagerInfo{};
	graphicPipelineManagerInfo.getInputLayoutFunc =
		[&](IDxcBlob* shaderBlob) { return shaderReflection_->GetInputLayoutElement(shaderBlob); };
	graphicPipelineManagerInfo.getRootParameterFunc =
		[&](IDxcBlob* shaderBlob) { return shaderReflection_->GetRootParameterElement(shaderBlob); };
	graphicPipelineManagerInfo.compileFunc =
		[&](const std::wstring& filePath, const wchar_t* profile) { return shaderCompiler_->CompileShader(filePath, profile); };
	graphicPipelineManagerInfo.getRenderTargetCountFunc =
		[&](IDxcBlob* shaderBlob) { return shaderReflection_->GetRenderTargetCount(shaderBlob); };
	graphicPipelineManagerInfo.device = directXDevice_->GetDevice();
	graphicPipelineManager_->Initialize(graphicPipelineManagerInfo);

	// Computeパイプラインマネージャの初期化
	ComputePipelineManagerInitializeInfo computePipelineManagerInfo{};
	computePipelineManagerInfo.getRootParameterFunc =
		[&](IDxcBlob* shaderBlob) { return shaderReflection_->GetRootParameterElement(shaderBlob); };
	computePipelineManagerInfo.getThreadGroupSizeFunc =
		[&](IDxcBlob* shaderBlob, UINT& sizeX, UINT& sizeY, UINT& sizeZ) { return shaderReflection_->GetThreadGroupSize(shaderBlob, sizeX, sizeY, sizeZ); };
	computePipelineManagerInfo.compileFunc =
		[&](const std::wstring& filePath, const wchar_t* profile) { return shaderCompiler_->CompileShader(filePath, profile); };
	computePipelineManagerInfo.device = directXDevice_->GetDevice();
	computePipelineManager_->Initialize(computePipelineManagerInfo);

	// RayTracingパイプラインマネージャの初期化
	RaytracingPipelineManagerInitializeInfo rayTracingPipelineManagerInfo{};
	rayTracingPipelineManagerInfo.compileFunc =
		[&](const std::wstring& filePath, const wchar_t* profile) { return shaderCompiler_->CompileShader(filePath, profile); };
	rayTracingPipelineManagerInfo.getRootParameterFunc =
		[&](IDxcBlob* shaderBlob) { return shaderLibReflection_->GetRootParameterElement(shaderBlob); };
	rayTracingPipelineManagerInfo.device = directXDevice_->GetDevice5();
	rayTracingPipelineManager_->Initialize(rayTracingPipelineManagerInfo);

	// テクスチャ管理クラスの初期化
	TextureLoaderInitializeInfo textureLoaderInfo{};
	// テクスチャのリソース作成やSRV作成、データアップロードに必要な情報をTextureLoaderInitializeInfo構造体にセット
	textureLoaderInfo.device = directXDevice_->GetDevice();
	// テクスチャのリソース作成に必要な関数をDirectXResourceContainerのCreateResource関数を呼び出すラムダ式で初期化
	textureLoaderInfo.createResourceFunc =
		[&](const D3D12_RESOURCE_DESC& desc) { return resourceContainer_->CreateResource(directXDevice_->GetDevice(), desc, D3D12_RESOURCE_STATE_GENERIC_READ); };
	// テクスチャのSRV作成に必要な関数をDirectXResourceContainerのCreateResourceView関数を呼び出すラムダ式で初期化
	textureLoaderInfo.createShaderResourceViewFunc =
		[&](DirectXResourceHandle resourceHandle, const D3D12_SHADER_RESOURCE_VIEW_DESC& desc) {
		CreateViewInfo createViewInfo{};
		createViewInfo.viewType = ViewTypeFlags::ShaderResourceView;
		createViewInfo.dsvDesc = {};
		createViewInfo.rtvDesc = {};
		createViewInfo.srvDesc = desc;
		createViewInfo.uavDesc = {};
		resourceContainer_->CreateResourceView(resourceHandle, createViewInfo); };
	// テクスチャのアップロードに必要な関数をDirectXResourceContainerのUploadResource関数を呼び出すラムダ式で初期化
	textureLoaderInfo.uploadTextureDataFunc =
		[&](DirectXResourceHandle resourceHandle, const std::vector<D3D12_SUBRESOURCE_DATA>& subresources) {
		resourceContainer_->UploadResource(resourceHandle, subresources, directXDevice_->GetDevice(), commandManager_->GetCommandList(D3D12_COMMAND_LIST_TYPE_DIRECT));
		};
	// テクスチャローダーを初期化
	textureLoader_->Initialize(textureLoaderInfo);

	// レイトレーシング用のアクセラレーション構造体の初期化
	accelerationStructure_.Initialize(directXDevice_->GetDevice5());
}

void D3D12GraphicEngine::PreDraw() {
	

	// ディスクリプタヒープの登録
	descriptorHeapManager_->RegisterDescriptorHeaps(commandManager_->GetCommandList(D3D12_COMMAND_LIST_TYPE_DIRECT));

	// スワップチェーンのリソース状態を描画可能に変更
	renderPass_->PreDraw(commandManager_->GetCommandList(D3D12_COMMAND_LIST_TYPE_DIRECT));

	// デプスステンシルのクリア
	ClearDepthStencil(depthStencilBufferHandle_);
}

void D3D12GraphicEngine::PostDraw() {
	// スワップチェーンのリソース状態を表示可能に変更
	renderPass_->PostDraw(commandManager_->GetCommandList(D3D12_COMMAND_LIST_TYPE_DIRECT));

	// コマンドリストをクローズ、実行
	commandManager_->ExecuteCommandList();

	// スワップチェーンの画面への表示
	renderPass_->Present();

	// GPUとの同期
	fence_->Signal(commandManager_->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT));
	fence_->Wait();
	// コマンドリストをリセット
	commandManager_->ResetCommandList();

	// 中間リソースの解放
	resourceContainer_->EndFrame();

	// プールの解放
	resourceAllocator_->ResetFrame();
}

void D3D12GraphicEngine::Shutdown() {
	rayTracingPipelineManager_->Finalize();
	textureLoader_->Finalize();
	fence_->Shutdown();
	shaderCompiler_->Finalize();
	directXDevice_->Shutdown();
}

DirectXDevice* QFE::GRAPHIC::D3D12GraphicEngine::GetDirectXDevice() const {
	assert(directXDevice_ != nullptr);
	return directXDevice_.get();
}

DirectXResourceContainer* QFE::GRAPHIC::D3D12GraphicEngine::GetDirectXResourceContainer() const {
	assert(resourceContainer_ != nullptr);
	return resourceContainer_.get();
}

DirectXResourceAllocator* QFE::GRAPHIC::D3D12GraphicEngine::GetDirectXResourceAllocator() const {
	assert(resourceAllocator_ != nullptr);
	return resourceAllocator_.get();
}

DescriptorHeapManager* QFE::GRAPHIC::D3D12GraphicEngine::GetDescriptorHeapManager() const {
	assert(descriptorHeapManager_ != nullptr);
	return descriptorHeapManager_.get();
}

DirectXCommandManager* QFE::GRAPHIC::D3D12GraphicEngine::GetDirectXCommandManager() const {
	assert(commandManager_ != nullptr);	
	return commandManager_.get();
}

Fence* QFE::GRAPHIC::D3D12GraphicEngine::GetFence() const {
	assert(fence_ != nullptr);
	return fence_.get();
}

ShaderReflection* QFE::GRAPHIC::D3D12GraphicEngine::GetShaderReflection() const {
	assert(shaderReflection_ != nullptr);
	return shaderReflection_.get();
}

ShaderLibReflection* QFE::GRAPHIC::D3D12GraphicEngine::GetShaderLibReflection() const {
	assert(shaderLibReflection_ != nullptr);
	return shaderLibReflection_.get();
}

ShaderCompiler* QFE::GRAPHIC::D3D12GraphicEngine::GetShaderCompiler() const {
	assert(shaderCompiler_ != nullptr);
	return shaderCompiler_.get();
}

GraphicPipelineManager* QFE::GRAPHIC::D3D12GraphicEngine::GetGraphicPipelineManager() const {
	assert(graphicPipelineManager_ != nullptr);
	return graphicPipelineManager_.get();
}

ComputePipelineManager* QFE::GRAPHIC::D3D12GraphicEngine::GetComputePipelineManager() const {
	assert(computePipelineManager_ != nullptr);
	return computePipelineManager_.get();
}

RaytracingPipelineManager* QFE::GRAPHIC::D3D12GraphicEngine::GetRayTracingPipelineManager() const {
	assert(rayTracingPipelineManager_ != nullptr);
	return rayTracingPipelineManager_.get();
}

TextureLoader* QFE::GRAPHIC::D3D12GraphicEngine::GetTextureLoader() const {
	assert(textureLoader_ != nullptr);
	return textureLoader_.get();
}

QFE::UniqueContainer<D3D12_VIEWPORT>& QFE::GRAPHIC::D3D12GraphicEngine::GetViewports() {
	return viewports_;
}

QFE::UniqueContainer<D3D12_RECT>& QFE::GRAPHIC::D3D12GraphicEngine::GetScissorRects() {
	return scissorRects_;
}

RenderPass* QFE::GRAPHIC::D3D12GraphicEngine::GetRenderPass() const {
	assert(renderPass_ != nullptr);
	return renderPass_.get();
}

RaytracingAccelerationStructure* QFE::GRAPHIC::D3D12GraphicEngine::GetRaytracingAccelerationStructure() {
	return &accelerationStructure_;
}

DirectXResourceHandle QFE::GRAPHIC::D3D12GraphicEngine::GetDepthStencilBufferHandle() const {
	assert(depthStencilBufferHandle_ != DirectXResourceHandle::Invalid);
	return depthStencilBufferHandle_;
}

void D3D12GraphicEngine::LegacyInitialize(uint32_t width, uint32_t height) {
	depthStencilBufferHandle_ = CreateDepthStencilBuffer(width, height);
}

DirectXResourceHandle QFE::GRAPHIC::D3D12GraphicEngine::CreateDepthStencilBuffer(uint32_t width, uint32_t height) {
	// depthStencilBufferの生成
	D3D12_RESOURCE_DESC depthResourceDesc{};
	depthResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthResourceDesc.Alignment = 0;
	depthResourceDesc.Width = width;
	depthResourceDesc.Height = height;
	depthResourceDesc.DepthOrArraySize = 1;
	depthResourceDesc.MipLevels = 1;
	// ★テクスチャとしても扱えるよう、型無し(TYPELESS)フォーマットにする
	depthResourceDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	depthResourceDesc.SampleDesc.Count = 1;
	depthResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE depthClearValue{};
	// ★DSV用の明確なフォーマットを指定する
	depthClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthClearValue.DepthStencil.Depth = 1.0f;
	depthClearValue.DepthStencil.Stencil = 0;

	DirectXResourceHandle handle = resourceContainer_->CreateResource(
		directXDevice_->GetDevice(),
		depthResourceDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		D3D12_HEAP_TYPE_DEFAULT,
		&depthClearValue);

	// DSVの生成
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; // 完全なデプス用フォーマット
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

	// ★追加: SRV（テクスチャ読み込み用）の記述子設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	// 24bitのデプス値を赤(R)成分として読み込む
	srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MipLevels = 1;

	// ビューの生成情報を設定
	CreateViewInfo createViewInfo{};
	// ★DSVとSRVの両方のフラグを立てる
	createViewInfo.viewType = ViewTypeFlags::DepthStencilView | ViewTypeFlags::ShaderResourceView;
	createViewInfo.dsvDesc = dsvDesc;
	createViewInfo.srvDesc = srvDesc; // ★セット
	resourceContainer_->CreateResourceView(handle, createViewInfo);

	return handle;
}

void D3D12GraphicEngine::ClearDepthStencil(DirectXResourceHandle depthStencilHandle) {
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = resourceContainer_->GetDescriptorHandleCPU(depthStencilHandle, ViewTypeFlags::DepthStencilView);
	commandManager_->GetCommandList(D3D12_COMMAND_LIST_TYPE_DIRECT)->ClearDepthStencilView(
		dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
}