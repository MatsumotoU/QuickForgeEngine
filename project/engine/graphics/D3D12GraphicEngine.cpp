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
		CereateViewInfo viewInfo{};
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
		CereateViewInfo createViewInfo{};
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

ViewPortHandle D3D12GraphicEngine::CreateViewPort(uint32_t width, uint32_t height) {
	D3D12_VIEWPORT viewport{};
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	viewport.Width = static_cast<float>(width);
	viewport.Height = static_cast<float>(height);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	uint32_t handle = viewports_.Add(std::to_string(width) + "x" + std::to_string(height), viewport);
	return static_cast<ViewPortHandle>(handle);
}

ScissorRectHandle QFE::GRAPHIC::D3D12GraphicEngine::CreateScissorRect(int left, int top, int right, int bottom) {
	D3D12_RECT scissorRect{};
	scissorRect.left = left;
	scissorRect.top = top;
	scissorRect.right = right;
	scissorRect.bottom = bottom;

	uint32_t handle = scissorRects_.Add(std::to_string(left) + "_" + std::to_string(top) + "_" + std::to_string(right) + "_" + std::to_string(bottom), scissorRect);
	return static_cast<ScissorRectHandle>(handle);
}

DirectXResourceHandle QFE::GRAPHIC::D3D12GraphicEngine::CreateTextureFromFile(const std::string& filePath) {
	return textureLoader_->LoadTexture(filePath);
}

DirectXResourceHandle QFE::GRAPHIC::D3D12GraphicEngine::GetBuiltInTextureHandle(BuiltInTextureType type) {
	if (BuiltInTextureType::DummyBlackCubeMap == type) {
		return textureLoader_->GetDummyBlackCubeMapHandle();
	} else if (BuiltInTextureType::DummyWhite1x1Texture == type) {
		return textureLoader_->GetDummyWhite1x1TextureHandle();
	} else {
		return DirectXResourceHandle::Invalid;
	}
}

DirectXResourceHandle QFE::GRAPHIC::D3D12GraphicEngine::CreateVertexBuffer(const std::vector<VertexData>& vertexData, const std::string& meshName) {
	DirectXResourceHandle handle =
		resourceContainer_->CreateBuffer(directXDevice_->GetDevice(), vertexData.size() * sizeof(VertexData));
	QFE_LOG("VertexBuffer created for mesh: " + meshName + ", size: " + std::to_string(vertexData.size() * sizeof(VertexData)) + " bytes");
	// バッファのストライドを設定
	resourceContainer_->SetResourceStrideInBytes(handle, sizeof(VertexData));

	resourceContainer_->MapResource(handle);
	VertexData* mappedData = resourceContainer_->GetMappedData<VertexData>(handle);
	if (mappedData) {
		memcpy(mappedData, vertexData.data(), vertexData.size() * sizeof(VertexData));
	}
	resourceContainer_->SetResourceName(handle, ConvertString(meshName));
	resourceContainer_->SetResourceStrideInBytes(handle, sizeof(VertexData));
	return handle;
}

ShaderPairHandle D3D12GraphicEngine::CreateShaderPair(const ShaderPairElement& element) {
	ShaderPairHandle shaderPairHandle =
		graphicPipelineManager_->GenerateShaderPair(element, [&](const std::wstring& filePath, const wchar_t* profile) {
		return shaderCompiler_->CompileShader(filePath, profile);
			});
	return shaderPairHandle;
}

PSOHandle D3D12GraphicEngine::CreatePipelineStateObject(ShaderPairHandle shaderHandle, BlendMode blendMode, RasterizerType rasterizerType, DepthStencilDescType depthStencilDescType) {
	PSOHandle psoHandle = graphicPipelineManager_->GeneratePipelineStateObject(
		directXDevice_->GetDevice(),
		shaderHandle, blendMode, rasterizerType, depthStencilDescType);
	return psoHandle;
}

PSOHandle D3D12GraphicEngine::GetBuiltInPipelineStateObject(
	BuiltInShaderPair builtInShaderPair, BlendMode blendMode, RasterizerType rasterizerType, DepthStencilDescType depthStencilDescType) {

	PSOHandle psoHandle = graphicPipelineManager_->GetBuiltInPSOHandle(builtInShaderPair, blendMode, rasterizerType, depthStencilDescType);
	return psoHandle;
}

ComputePSOHandle D3D12GraphicEngine::CreateComputePipelineStateObject(
	const std::string& dirPath, const std::string& csFileName) {

	ComputePSOHandle computePSOHandle =
		computePipelineManager_->GenerateComputePipelineStateObject(dirPath, csFileName);
	return computePSOHandle;
}

RenderTargetHandle D3D12GraphicEngine::CreateOffScreenRenderTarget(uint32_t width, uint32_t height, DXGI_FORMAT format) {
	return renderPass_->CreateOffscreenRenderTarget(width, height, format);
}

size_t D3D12GraphicEngine::GetResourceArraySize(DirectXResourceHandle handle) {
	size_t stride = resourceContainer_->GetResourceStrideInBytes(handle);
	if (stride == 0) {
		QFE_LOG("Resource stride is zero for handle: " + std::to_string(static_cast<uint32_t>(handle)));
		return 0;
	}
	size_t size = resourceContainer_->GetResourceSizeInBytes(handle);
	return size / stride;
}

DirectXResourceAllocator* QFE::GRAPHIC::D3D12GraphicEngine::GetResourceAllocator() {
	return resourceAllocator_.get();
}

DirectXResourceHandle QFE::GRAPHIC::D3D12GraphicEngine::CreateUAVBuffer(uint32_t width, uint32_t height, const std::wstring& name) {
	D3D12_RESOURCE_DESC texDesc{};
	texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	texDesc.Width = static_cast<UINT>(width);                     // 画面の横幅
	texDesc.Height = static_cast<UINT>(height);                    // 画面の縦幅
	texDesc.DepthOrArraySize = 1;
	texDesc.MipLevels = 1;                     // CSで直接塗るためミップマップは1でOK
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // バックバッファと合わせておく
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

	// リソースを生成
	DirectXResourceHandle handle =
		resourceContainer_->CreateResource(
			directXDevice_->GetDevice(), texDesc, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_HEAP_TYPE_DEFAULT);

	// UAVの生成
	CereateViewInfo createViewInfo{};
	createViewInfo.viewType = ViewTypeFlags::UnorderedAccessView;
	createViewInfo.uavDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	createViewInfo.uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;

	resourceContainer_->CreateResourceView(handle, createViewInfo);
	resourceContainer_->SetResourceName(handle, name);

	return handle;
}

RTPSOHandle QFE::GRAPHIC::D3D12GraphicEngine::CreateRayTracingPipelineStateObject(const std::string& dirPath, const std::string& rgsFileName) {
	return rayTracingPipelineManager_->CreateRaytracingPipelineStateObject(ConvertString(dirPath + rgsFileName), L"lib_6_3");
}

BLASHandle QFE::GRAPHIC::D3D12GraphicEngine::CreateBLAS(std::vector<QFE::MATH::Vector3> vertices, const std::string& name) {
	return accelerationStructure_.CreateBLAS(
		directXDevice_->GetDevice5(), commandManager_->GetCommandList4(D3D12_COMMAND_LIST_TYPE_DIRECT), vertices, name);
}

void QFE::GRAPHIC::D3D12GraphicEngine::UpdateBLASInstanceTransform(const std::vector<QFE::GRAPHIC::RaytracingInstance>& instances) {
	// TLASの更新
	accelerationStructure_.UpdateTLAS(commandManager_->GetCommandList4(D3D12_COMMAND_LIST_TYPE_DIRECT), instances);
	commandManager_->ExecuteCommandList();
	fence_->Signal(commandManager_->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT));
	fence_->Wait();
	commandManager_->ResetCommandList();
}

DirectXResourceHandle QFE::GRAPHIC::D3D12GraphicEngine::GetRenderTargetTexture(RenderTargetHandle renderTargetHandle) {
	return renderPass_->GetOffscreenBarrierShaderResourceHandle(renderTargetHandle);
}

void QFE::GRAPHIC::D3D12GraphicEngine::SetRenderTarget(RenderTargetHandle renderTargetHandle) {
	renderPass_->SetRenderTarget(commandManager_->GetCommandList(D3D12_COMMAND_LIST_TYPE_DIRECT), depthStencilBufferHandle_, renderTargetHandle);
}

void D3D12GraphicEngine::TestDraw(
	PSOHandle psoHandle, ViewPortHandle viewportHandle, ScissorRectHandle scissorRectHandle,
	DirectXResourceHandle vertexBufferHandle, std::vector<DirectXResourceHandle> rootResources) {

	ID3D12GraphicsCommandList* commandList = commandManager_->GetCommandList(D3D12_COMMAND_LIST_TYPE_DIRECT);

	renderPass_->SetRenderTarget(
		commandList, depthStencilBufferHandle_, RenderTargetHandle::SwapChain);

	commandList->RSSetViewports(1, viewports_.GetData(static_cast<uint32_t>(viewportHandle)));
	commandList->RSSetScissorRects(1, scissorRects_.GetData(static_cast<uint32_t>(scissorRectHandle)));

	commandList->SetPipelineState(graphicPipelineManager_->GetPipelineState(psoHandle));
	commandList->SetGraphicsRootSignature(graphicPipelineManager_->GetRootSignature(psoHandle));

	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView = resourceContainer_->GetVertexBufferView(vertexBufferHandle);
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView);

	std::vector<D3D12_ROOT_PARAMETER_TYPE> rootParameterTypes = graphicPipelineManager_->GetRootParameterTypes(psoHandle);
	if (rootParameterTypes.size() != rootResources.size()) {
		// PSOのルートパラメータの数と渡されたリソースの数が異なる場合はエラー
		assert(false);
		return;
	}

	for (int i = 0; i < rootParameterTypes.size(); ++i) {
		D3D12_ROOT_PARAMETER_TYPE rootParameterType = rootParameterTypes[i];

		if (rootParameterType == D3D12_ROOT_PARAMETER_TYPE_CBV) {
			D3D12_GPU_VIRTUAL_ADDRESS gpuHandle = resourceContainer_->GetGpuVirtualAddress(rootResources[i]);
			commandList->SetGraphicsRootConstantBufferView(static_cast<UINT>(i), gpuHandle);
		} else {
			D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = resourceContainer_->GetDescriptorHandleGPU(rootResources[i], rootParameterType);
			switch (rootParameterType) {
			case D3D12_ROOT_PARAMETER_TYPE_SRV:
				commandList->SetGraphicsRootDescriptorTable(static_cast<UINT>(i), gpuHandle);
				break;
			case D3D12_ROOT_PARAMETER_TYPE_UAV:
				commandList->SetGraphicsRootUnorderedAccessView(static_cast<UINT>(i), gpuHandle.ptr);
				break;
			case D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE:
				commandList->SetGraphicsRootDescriptorTable(static_cast<UINT>(i), gpuHandle);
				break;
			default:
				break;
			}
		}


	}

	UINT vertexCount = static_cast<UINT>(GetResourceArraySize(vertexBufferHandle));
	commandList->DrawInstanced(vertexCount, 1, 0, 0);
}

void QFE::GRAPHIC::D3D12GraphicEngine::TestOffScreenDraw(
	PSOHandle psoHandle, ViewPortHandle viewportHandle, ScissorRectHandle scissorRectHandle,
	DirectXResourceHandle vertexBufferHandle, std::vector<DirectXResourceHandle> rootResources,
	std::vector<RenderTargetHandle> renderTargets) {

	ID3D12GraphicsCommandList* commandList = commandManager_->GetCommandList(D3D12_COMMAND_LIST_TYPE_DIRECT);

	renderPass_->SetRenderTarget(
		commandList, depthStencilBufferHandle_, renderTargets);

	commandList->RSSetViewports(1, viewports_.GetData(static_cast<uint32_t>(viewportHandle)));
	commandList->RSSetScissorRects(1, scissorRects_.GetData(static_cast<uint32_t>(scissorRectHandle)));

	commandList->SetPipelineState(graphicPipelineManager_->GetPipelineState(psoHandle));
	commandList->SetGraphicsRootSignature(graphicPipelineManager_->GetRootSignature(psoHandle));

	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView = resourceContainer_->GetVertexBufferView(vertexBufferHandle);
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView);

	std::vector<D3D12_ROOT_PARAMETER_TYPE> rootParameterTypes = graphicPipelineManager_->GetRootParameterTypes(psoHandle);
	if (rootParameterTypes.size() != rootResources.size()) {
		// PSOのルートパラメータの数と渡されたリソースの数が異なる場合はエラー
		assert(false);
		return;
	}

	for (int i = 0; i < rootParameterTypes.size(); ++i) {
		D3D12_ROOT_PARAMETER_TYPE rootParameterType = rootParameterTypes[i];

		if (rootParameterType == D3D12_ROOT_PARAMETER_TYPE_CBV) {
			D3D12_GPU_VIRTUAL_ADDRESS gpuHandle = resourceContainer_->GetGpuVirtualAddress(rootResources[i]);
			commandList->SetGraphicsRootConstantBufferView(static_cast<UINT>(i), gpuHandle);
		} else {
			D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = resourceContainer_->GetDescriptorHandleGPU(rootResources[i], rootParameterType);
			switch (rootParameterType) {
			case D3D12_ROOT_PARAMETER_TYPE_SRV:
				commandList->SetGraphicsRootDescriptorTable(static_cast<UINT>(i), gpuHandle);
				break;
			case D3D12_ROOT_PARAMETER_TYPE_UAV:
				commandList->SetGraphicsRootUnorderedAccessView(static_cast<UINT>(i), gpuHandle.ptr);
				break;
			case D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE:
				commandList->SetGraphicsRootDescriptorTable(static_cast<UINT>(i), gpuHandle);
				break;
			default:
				break;
			}
		}
	}

	UINT vertexCount = static_cast<UINT>(GetResourceArraySize(vertexBufferHandle));
	commandList->DrawInstanced(vertexCount, 1, 0, 0);
}

void D3D12GraphicEngine::TestCompute(ComputePSOHandle computePSOHandle, DirectXResourceHandle uavHandle, DirectXResourceHandle constantBufferHandle) {
	ID3D12GraphicsCommandList* commandList = commandManager_->GetCommandList(D3D12_COMMAND_LIST_TYPE_DIRECT);
	resourceContainer_->TransitionResource(uavHandle, commandList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	commandList->SetPipelineState(computePipelineManager_->GetPipelineState(computePSOHandle));
	commandList->SetComputeRootSignature(computePipelineManager_->GetRootSignature(computePSOHandle));

	// UAVバッファをルートパラメータにバインド
	D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = resourceContainer_->GetDescriptorHandleGPU(uavHandle, QFE::GRAPHIC::ViewTypeFlags::UnorderedAccessView);
	commandList->SetComputeRootDescriptorTable(1, gpuHandle);

	D3D12_GPU_VIRTUAL_ADDRESS constantBufferGPUAddress = resourceContainer_->GetGpuVirtualAddress(constantBufferHandle);
	commandList->SetComputeRootConstantBufferView(0, constantBufferGPUAddress);

	UINT threadGroupSizeX, threadGroupSizeY, threadGroupSizeZ;
	computePipelineManager_->GetThreadGroupSize(computePSOHandle, threadGroupSizeX, threadGroupSizeY, threadGroupSizeZ);

	UINT resourceWidth = resourceContainer_->GetResourceWidth(uavHandle);
	UINT resourceHeight = resourceContainer_->GetResourceHeight(uavHandle);
	commandList->Dispatch(resourceWidth / threadGroupSizeX, resourceHeight / threadGroupSizeY, threadGroupSizeZ);

	resourceContainer_->TransitionResource(uavHandle, commandList, D3D12_RESOURCE_STATE_COPY_SOURCE);

	renderPass_->TransitionCurrentBackBufferBarrier(
		commandList, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_DEST);

	commandList->CopyResource(
		renderPass_->GetCurrentBackBuffer(), resourceContainer_->GetResource(uavHandle));

	renderPass_->TransitionCurrentBackBufferBarrier(
		commandList, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_RENDER_TARGET);
}

void QFE::GRAPHIC::D3D12GraphicEngine::TestRayTracing(RTPSOHandle rtpsoHandle, DirectXResourceHandle uavHandle) {
	ID3D12GraphicsCommandList4* commandList4 = commandManager_->GetCommandList4(D3D12_COMMAND_LIST_TYPE_DIRECT);
	ID3D12RootSignature* globalRootSignature = rayTracingPipelineManager_->GetRaytracingPipelineStateObject(rtpsoHandle)->GetRootSignature();
	ID3D12StateObject* rtpsoptr = rayTracingPipelineManager_->GetRaytracingPipelineStateObject(rtpsoHandle)->GetPipelineState();
	ID3D12Resource* rayGenShaderTable_ = rayTracingPipelineManager_->GetRaytracingPipelineStateObject(rtpsoHandle)->GetRayGenShaderTable();
	ID3D12Resource* missShaderTable_ = rayTracingPipelineManager_->GetRaytracingPipelineStateObject(rtpsoHandle)->GetMissShaderTable();
	ID3D12Resource* hitGroupShaderTable_ = rayTracingPipelineManager_->GetRaytracingPipelineStateObject(rtpsoHandle)->GetHitGroupShaderTable();

	ID3D12GraphicsCommandList* commandList = commandManager_->GetCommandList(D3D12_COMMAND_LIST_TYPE_DIRECT);
	resourceContainer_->TransitionResource(uavHandle, commandList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	// 1. DXR用のパイプライン(RTPSO)とルートシグネチャをコマンドリストにセット
	// (マネージャー等で生成したオブジェクトを設定します)
	commandList4->SetComputeRootSignature(globalRootSignature);
	commandList4->SetPipelineState1(rtpsoptr); // ★レイトレPSOはSetPipelineState1を使う

	// 2. 自動化されたルートシグネチャへのリソースバインド
	D3D12_GPU_VIRTUAL_ADDRESS tlasResultBufferGPUHandle = accelerationStructure_.GetTLASResultBuffer()->GetGPUVirtualAddress();
	commandList4->SetComputeRootShaderResourceView(0, tlasResultBufferGPUHandle);
	D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = resourceContainer_->GetDescriptorHandleGPU(uavHandle, QFE::GRAPHIC::ViewTypeFlags::UnorderedAccessView);
	commandList4->SetComputeRootDescriptorTable(1, gpuHandle);

	// 3. シェーダーレコードのサイズ定義（前段で作った64バイトと同じ）
	const UINT shaderRecordSize = (D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES + D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT - 1)
		& ~(D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT - 1); // 64

	// 4. DispatchRays の設定構造体を埋める
	D3D12_DISPATCH_RAYS_DESC dispatchDesc{};

	// --- RayGeneration テーブルの指定 ---
	dispatchDesc.RayGenerationShaderRecord.StartAddress = rayGenShaderTable_->GetGPUVirtualAddress();
	dispatchDesc.RayGenerationShaderRecord.SizeInBytes = shaderRecordSize; // 64バイト

	// --- Miss テーブルの指定 ---
	dispatchDesc.MissShaderTable.StartAddress = missShaderTable_->GetGPUVirtualAddress();
	dispatchDesc.MissShaderTable.SizeInBytes = shaderRecordSize; // 1つ分なので64バイト
	dispatchDesc.MissShaderTable.StrideInBytes = shaderRecordSize; // 1つあたりの歩進サイズ

	// --- HitGroup テーブルの指定（今回はまだ空なので0） ---
	dispatchDesc.HitGroupTable.StartAddress = hitGroupShaderTable_->GetGPUVirtualAddress();
	dispatchDesc.HitGroupTable.SizeInBytes = shaderRecordSize; // 1つのレコードサイズ
	dispatchDesc.HitGroupTable.StrideInBytes = shaderRecordSize;

	// --- Callable テーブルの指定（使わないので0） ---
	dispatchDesc.CallableShaderTable.StartAddress = 0;
	dispatchDesc.CallableShaderTable.SizeInBytes = 0;
	dispatchDesc.CallableShaderTable.StrideInBytes = 0;

	// --- 追跡する画面の解像度を指定（このピクセル数分の光線が一斉に飛びます） ---
	dispatchDesc.Width = 1280;  // 例: 1920
	dispatchDesc.Height = 720; // 例: 1080
	dispatchDesc.Depth = 1;            // 2D画面なので 1

	// 5. ★運命のコマンド発行！
	commandList4->DispatchRays(&dispatchDesc);


	// 6. スワップチェーンのバックバッファにコピー
	resourceContainer_->TransitionResource(uavHandle, commandList, D3D12_RESOURCE_STATE_COPY_SOURCE);
	renderPass_->TransitionCurrentBackBufferBarrier(
		commandList, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_DEST);
	commandList->CopyResource(
		renderPass_->GetCurrentBackBuffer(), resourceContainer_->GetResource(uavHandle));
	renderPass_->TransitionCurrentBackBufferBarrier(
		commandList, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_RENDER_TARGET);
}

void QFE::GRAPHIC::D3D12GraphicEngine::TestRayTracing(
	RTPSOHandle rtpsoHandle, DirectXResourceHandle uavHandle, std::vector<DirectXResourceHandle> rootResources) {

	ID3D12GraphicsCommandList4* commandList4 = commandManager_->GetCommandList4(D3D12_COMMAND_LIST_TYPE_DIRECT);
	ID3D12RootSignature* globalRootSignature = rayTracingPipelineManager_->GetRaytracingPipelineStateObject(rtpsoHandle)->GetRootSignature();
	ID3D12StateObject* rtpsoptr = rayTracingPipelineManager_->GetRaytracingPipelineStateObject(rtpsoHandle)->GetPipelineState();
	ID3D12Resource* rayGenShaderTable_ = rayTracingPipelineManager_->GetRaytracingPipelineStateObject(rtpsoHandle)->GetRayGenShaderTable();
	ID3D12Resource* missShaderTable_ = rayTracingPipelineManager_->GetRaytracingPipelineStateObject(rtpsoHandle)->GetMissShaderTable();
	ID3D12Resource* hitGroupShaderTable_ = rayTracingPipelineManager_->GetRaytracingPipelineStateObject(rtpsoHandle)->GetHitGroupShaderTable();

	ID3D12GraphicsCommandList* commandList = commandManager_->GetCommandList(D3D12_COMMAND_LIST_TYPE_DIRECT);
	resourceContainer_->TransitionResource(uavHandle, commandList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	// 1. DXR用のパイプライン(RTPSO)とルートシグネチャをコマンドリストにセット
	// (マネージャー等で生成したオブジェクトを設定します)
	commandList4->SetComputeRootSignature(globalRootSignature);
	commandList4->SetPipelineState1(rtpsoptr); // ★レイトレPSOはSetPipelineState1を使う

	// 2. 自動化されたルートシグネチャへのリソースバインド
	D3D12_GPU_VIRTUAL_ADDRESS tlasResultBufferGPUHandle = accelerationStructure_.GetTLASResultBuffer()->GetGPUVirtualAddress();
	commandList4->SetComputeRootShaderResourceView(0, tlasResultBufferGPUHandle);
	D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = resourceContainer_->GetDescriptorHandleGPU(uavHandle, QFE::GRAPHIC::ViewTypeFlags::UnorderedAccessView);

	commandList4->SetComputeRootDescriptorTable(1, resourceContainer_->GetDescriptorHandleGPU(rootResources[0], QFE::GRAPHIC::ViewTypeFlags::ShaderResourceView));
	commandList4->SetComputeRootDescriptorTable(2, resourceContainer_->GetDescriptorHandleGPU(rootResources[1], QFE::GRAPHIC::ViewTypeFlags::ShaderResourceView));
	commandList4->SetComputeRootDescriptorTable(3, resourceContainer_->GetDescriptorHandleGPU(rootResources[2], QFE::GRAPHIC::ViewTypeFlags::ShaderResourceView));
	commandList4->SetComputeRootDescriptorTable(4, gpuHandle);

	// 3. シェーダーレコードのサイズ定義（前段で作った64バイトと同じ）
	const UINT shaderRecordSize = (D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES + D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT - 1)
		& ~(D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT - 1); // 64

	// 4. DispatchRays の設定構造体を埋める
	D3D12_DISPATCH_RAYS_DESC dispatchDesc{};

	// --- RayGeneration テーブルの指定 ---
	dispatchDesc.RayGenerationShaderRecord.StartAddress = rayGenShaderTable_->GetGPUVirtualAddress();
	dispatchDesc.RayGenerationShaderRecord.SizeInBytes = shaderRecordSize; // 64バイト

	// --- Miss テーブルの指定 ---
	dispatchDesc.MissShaderTable.StartAddress = missShaderTable_->GetGPUVirtualAddress();
	dispatchDesc.MissShaderTable.SizeInBytes = shaderRecordSize; // 1つ分なので64バイト
	dispatchDesc.MissShaderTable.StrideInBytes = shaderRecordSize; // 1つあたりの歩進サイズ

	// --- HitGroup テーブルの指定（今回はまだ空なので0） ---
	dispatchDesc.HitGroupTable.StartAddress = hitGroupShaderTable_->GetGPUVirtualAddress();
	dispatchDesc.HitGroupTable.SizeInBytes = shaderRecordSize; // 1つのレコードサイズ
	dispatchDesc.HitGroupTable.StrideInBytes = shaderRecordSize;

	// --- Callable テーブルの指定（使わないので0） ---
	dispatchDesc.CallableShaderTable.StartAddress = 0;
	dispatchDesc.CallableShaderTable.SizeInBytes = 0;
	dispatchDesc.CallableShaderTable.StrideInBytes = 0;

	// --- 追跡する画面の解像度を指定（このピクセル数分の光線が一斉に飛びます） ---
	dispatchDesc.Width = 1280;  // 例: 1920
	dispatchDesc.Height = 720; // 例: 1080
	dispatchDesc.Depth = 1;            // 2D画面なので 1

	// 5. ★運命のコマンド発行！
	commandList4->DispatchRays(&dispatchDesc);

	// 6. スワップチェーンのバックバッファにコピー
	resourceContainer_->TransitionResource(uavHandle, commandList, D3D12_RESOURCE_STATE_COPY_SOURCE);
	renderPass_->TransitionCurrentBackBufferBarrier(
		commandList, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_DEST);
	commandList->CopyResource(
		renderPass_->GetCurrentBackBuffer(), resourceContainer_->GetResource(uavHandle));
	renderPass_->TransitionCurrentBackBufferBarrier(
		commandList, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_RENDER_TARGET);
}

void QFE::GRAPHIC::D3D12GraphicEngine::RayTracingDispatch(
	RTPSOHandle rtpsoHandle, DirectXResourceHandle uavHandle, 
	std::vector<DirectXResourceHandle> rootResources, DirectXResourceHandle copyResourceHandle) {

	ID3D12GraphicsCommandList4* commandList4 = commandManager_->GetCommandList4(D3D12_COMMAND_LIST_TYPE_DIRECT);
	ID3D12RootSignature* globalRootSignature = rayTracingPipelineManager_->GetRaytracingPipelineStateObject(rtpsoHandle)->GetRootSignature();
	ID3D12StateObject* rtpsoptr = rayTracingPipelineManager_->GetRaytracingPipelineStateObject(rtpsoHandle)->GetPipelineState();
	ID3D12Resource* rayGenShaderTable_ = rayTracingPipelineManager_->GetRaytracingPipelineStateObject(rtpsoHandle)->GetRayGenShaderTable();
	ID3D12Resource* missShaderTable_ = rayTracingPipelineManager_->GetRaytracingPipelineStateObject(rtpsoHandle)->GetMissShaderTable();
	ID3D12Resource* hitGroupShaderTable_ = rayTracingPipelineManager_->GetRaytracingPipelineStateObject(rtpsoHandle)->GetHitGroupShaderTable();

	ID3D12GraphicsCommandList* commandList = commandManager_->GetCommandList(D3D12_COMMAND_LIST_TYPE_DIRECT);
	resourceContainer_->TransitionResource(uavHandle, commandList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	// 1. DXR用のパイプライン(RTPSO)とルートシグネチャをコマンドリストにセット
	// (マネージャー等で生成したオブジェクトを設定します)
	commandList4->SetComputeRootSignature(globalRootSignature);
	commandList4->SetPipelineState1(rtpsoptr); // ★レイトレPSOはSetPipelineState1を使う

	// 2. 自動化されたルートシグネチャへのリソースバインド
	D3D12_GPU_VIRTUAL_ADDRESS tlasResultBufferGPUHandle = accelerationStructure_.GetTLASResultBuffer()->GetGPUVirtualAddress();
	commandList4->SetComputeRootShaderResourceView(0, tlasResultBufferGPUHandle);
	D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = resourceContainer_->GetDescriptorHandleGPU(uavHandle, QFE::GRAPHIC::ViewTypeFlags::UnorderedAccessView);

	commandList4->SetComputeRootDescriptorTable(1, resourceContainer_->GetDescriptorHandleGPU(rootResources[0], QFE::GRAPHIC::ViewTypeFlags::ShaderResourceView));
	commandList4->SetComputeRootDescriptorTable(2, resourceContainer_->GetDescriptorHandleGPU(rootResources[1], QFE::GRAPHIC::ViewTypeFlags::ShaderResourceView));
	commandList4->SetComputeRootDescriptorTable(3, resourceContainer_->GetDescriptorHandleGPU(rootResources[2], QFE::GRAPHIC::ViewTypeFlags::ShaderResourceView));
	commandList4->SetComputeRootDescriptorTable(4, gpuHandle);

	// 3. シェーダーレコードのサイズ定義（前段で作った64バイトと同じ）
	const UINT shaderRecordSize = (D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES + D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT - 1)
		& ~(D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT - 1); // 64

	// 4. DispatchRays の設定構造体を埋める
	D3D12_DISPATCH_RAYS_DESC dispatchDesc{};

	// --- RayGeneration テーブルの指定 ---
	dispatchDesc.RayGenerationShaderRecord.StartAddress = rayGenShaderTable_->GetGPUVirtualAddress();
	dispatchDesc.RayGenerationShaderRecord.SizeInBytes = shaderRecordSize; // 64バイト

	// --- Miss テーブルの指定 ---
	dispatchDesc.MissShaderTable.StartAddress = missShaderTable_->GetGPUVirtualAddress();
	dispatchDesc.MissShaderTable.SizeInBytes = shaderRecordSize; // 1つ分なので64バイト
	dispatchDesc.MissShaderTable.StrideInBytes = shaderRecordSize; // 1つあたりの歩進サイズ

	// --- HitGroup テーブルの指定（今回はまだ空なので0） ---
	dispatchDesc.HitGroupTable.StartAddress = hitGroupShaderTable_->GetGPUVirtualAddress();
	dispatchDesc.HitGroupTable.SizeInBytes = shaderRecordSize; // 1つのレコードサイズ
	dispatchDesc.HitGroupTable.StrideInBytes = shaderRecordSize;

	// --- Callable テーブルの指定（使わないので0） ---
	dispatchDesc.CallableShaderTable.StartAddress = 0;
	dispatchDesc.CallableShaderTable.SizeInBytes = 0;
	dispatchDesc.CallableShaderTable.StrideInBytes = 0;

	// --- 追跡する画面の解像度を指定（このピクセル数分の光線が一斉に飛びます） ---
	dispatchDesc.Width = 1280;  // 例: 1920
	dispatchDesc.Height = 720; // 例: 1080
	dispatchDesc.Depth = 1;            // 2D画面なので 1

	// 5. ★運命のコマンド発行！
	commandList4->DispatchRays(&dispatchDesc);

	// 6. コピー先リソースにコピー
	resourceContainer_->TransitionResource(uavHandle, commandList, D3D12_RESOURCE_STATE_COPY_SOURCE);
	resourceContainer_->TransitionResource(copyResourceHandle, commandList, D3D12_RESOURCE_STATE_COPY_DEST);
	commandList->CopyResource(
		resourceContainer_->GetResource(copyResourceHandle), resourceContainer_->GetResource(uavHandle));
	resourceContainer_->TransitionResource(copyResourceHandle, commandList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}

ID3D12Device* QFE::GRAPHIC::D3D12GraphicEngine::GetDevice() const {
	return directXDevice_->GetDevice();
}

ID3D12GraphicsCommandList* QFE::GRAPHIC::D3D12GraphicEngine::GetCommandList(D3D12_COMMAND_LIST_TYPE type) const {
	return commandManager_->GetCommandList(type);
}

UINT QFE::GRAPHIC::D3D12GraphicEngine::GetSwapChainBufferCount() const {
	return renderPass_->GetSwapChainBufferCount();
}

ID3D12DescriptorHeap* QFE::GRAPHIC::D3D12GraphicEngine::GetSRVDescriptorHeap() const {
	return descriptorHeapManager_->GetDescriptorHeap(GRAPHIC::DescriptorHeapType::SRV);
}

DescriptorHandles QFE::GRAPHIC::D3D12GraphicEngine::CreateExternalSRVDescriptor() {
	return descriptorHeapManager_->CreateEmptyHeapHandle(GRAPHIC::DescriptorHeapType::SRV);
}

D3D12_GPU_DESCRIPTOR_HANDLE QFE::GRAPHIC::D3D12GraphicEngine::GetSRVDescriptorGPUHandle(DirectXResourceHandle handle) const {
	return resourceContainer_->GetDescriptorHandleGPU(handle, ViewTypeFlags::ShaderResourceView);
}

D3D12_CPU_DESCRIPTOR_HANDLE QFE::GRAPHIC::D3D12GraphicEngine::GetSRVDescriptorCPUHandle(DirectXResourceHandle handle) const {
	return resourceContainer_->GetDescriptorHandleCPU(handle, ViewTypeFlags::ShaderResourceView);
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
	CereateViewInfo createViewInfo{};
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