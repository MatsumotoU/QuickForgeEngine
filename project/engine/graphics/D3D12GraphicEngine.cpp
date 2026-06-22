#include "D3D12GraphicEngine.h"
#include "EngineDefines.h"

#include "dx12/checker/DirectX12DebugCore.h"

#include "dx12/vram/descriptors/DescriptorHeapManager.h"
#include "dx12/command/DirectXCommandManager.h"
#include "dx12/RenderPass.h"
#include "dx12/Fence.h"

#include "dx12/pipeline/pso/ShaderCompiler.h"
#include "dx12/pipeline/pso/ShaderReflection.h"

#include "dx12/pipeline/GraphicPipelineManager.h"
#include "dx12/pipeline/ComputePipelineManager.h"
#include "dx12/TextureLoader.h"

#include "dx12/vram/descriptors/DescriptorHandles.h"

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
	computePipelineManager_(std::make_unique<ComputePipelineManager>())

{}

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
		{return DescriptorHandles{}; };
	resourceContainer_->Initialize(resourceContainerInfo);

	// RenderPassの初期化設定
	RenderPassInitializeInfo renderPassInfo{};
	renderPassInfo.width = width;
	renderPassInfo.height = height;
	renderPassInfo.hwnd = hwnd_;
	renderPassInfo.device = directXDevice_->GetDevice();
	renderPassInfo.dxgiFactory = directXDevice_->GetDxgiFactory();
	renderPassInfo.commandQueue = commandManager_->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
	// Dsvを取得する関数
	renderPassInfo.getResourceDsvFunc = [&](DirectXResourceHandle resourceHandle)
		{return resourceContainer_->GetDescriptorHandleCpuPtr(resourceHandle, ViewTypeFlags::DepthStencilView); };
	// Rtvに割り当てる関数
	renderPassInfo.assginRtvFunc = [&](ID3D12Resource* resource, const D3D12_RENDER_TARGET_VIEW_DESC* desc)
		{return descriptorHeapManager_->AssignRtvHeap(directXDevice_->GetDevice(), resource, desc).cpuHandle_; };
	// RenderPassの初期化
	renderPass_->Initialize(renderPassInfo);

	// Fenceの初期化
	fence_->Initialize(directXDevice_->GetDevice());

	// DirectXCommonの名残.フェンスの初期化以降の処理.
	LegacyInitialize(width, height);

	// グラフィックパイプラインマネージャの初期化
	GraphicPipelineManagerInitializeInfo graphicPipelineManagerInfo{};
	graphicPipelineManagerInfo.reflectionFunc = 
		[&](IDxcBlob* shaderBlob) { shaderReflection_->RunShaderReflection(shaderBlob); };
	graphicPipelineManagerInfo.getInputLayoutFunc = 
		[&](IDxcBlob* shaderBlob) { return shaderReflection_->GetInputLayoutElement(shaderBlob); };
	graphicPipelineManagerInfo.getRootParameterFunc = 
		[&](IDxcBlob* shaderBlob) { return shaderReflection_->GetRootParameterElement(shaderBlob); };
	graphicPipelineManagerInfo.compileFunc = 
		[&](const std::wstring& filePath, const wchar_t* profile) { return shaderCompiler_->CompileShader(filePath, profile); };
	graphicPipelineManagerInfo.device = directXDevice_->GetDevice();
	graphicPipelineManager_->Initialize(graphicPipelineManagerInfo);

	// Computeパイプラインマネージャの初期化
	ComputePipelineManagerInitializeInfo computePipelineManagerInfo{};
	computePipelineManagerInfo.reflectionFunc = 
		[&](IDxcBlob* shaderBlob) { shaderReflection_->RunShaderReflection(shaderBlob); };
	computePipelineManagerInfo.getRootParameterFunc = 
		[&](IDxcBlob* shaderBlob) { return shaderReflection_->GetRootParameterElement(shaderBlob); };
	computePipelineManagerInfo.getThreadGroupSizeFunc = 
		[&](IDxcBlob* shaderBlob, UINT& sizeX, UINT& sizeY, UINT& sizeZ) { return shaderReflection_->GetThreadGroupSize(shaderBlob, sizeX, sizeY, sizeZ); };
	computePipelineManagerInfo.compileFunc = 
		[&](const std::wstring& filePath, const wchar_t* profile) { return shaderCompiler_->CompileShader(filePath, profile); };
	computePipelineManagerInfo.device = directXDevice_->GetDevice();
	computePipelineManager_->Initialize(computePipelineManagerInfo);
	
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
		resourceContainer_->CreateResourceView(resourceHandle, createViewInfo);};
	// テクスチャのアップロードに必要な関数をDirectXResourceContainerのUploadResource関数を呼び出すラムダ式で初期化
	textureLoaderInfo.uploadTextureDataFunc = 
		[&](DirectXResourceHandle resourceHandle, const std::vector<D3D12_SUBRESOURCE_DATA>& subresources) {
		resourceContainer_->UploadResource(resourceHandle, subresources, directXDevice_->GetDevice(), commandManager_->GetCommandList(D3D12_COMMAND_LIST_TYPE_DIRECT));
		};
	// テクスチャローダーを初期化
	textureLoader_->Initialize(textureLoaderInfo);
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
}

void D3D12GraphicEngine::Shutdown() {
	textureLoader_->Finalize();
	fence_->Shutdown();
	shaderCompiler_->Finalize();
	directXDevice_->Shutdown();
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
	if(BuiltInTextureType::DummyBlackCubeMap == type) {
		return textureLoader_->GetDummyBlackCubeMapHandle();
	} else if(BuiltInTextureType::DummyWhite1x1Texture == type) {
		return textureLoader_->GetDummyWhite1x1TextureHandle();
	} else {
		return DirectXResourceHandle::Invalid;
	}
}

DirectXResourceHandle QFE::GRAPHIC::D3D12GraphicEngine::CreateVertexBuffer(const std::vector<VertexData>& vertexData, const std::string& meshName) {
	DirectXResourceHandle handle =
		resourceContainer_->CreateBuffer(directXDevice_->GetDevice(), vertexData.size() * sizeof(VertexData));
	// バッファのストライドを設定
	resourceContainer_->SetResourceStrideInBytes(handle, sizeof(VertexData));

	resourceContainer_->MapResource(handle);
	VertexData* mappedData = resourceContainer_->GetMappedData<VertexData>(handle);
	if (mappedData) {
		memcpy(mappedData, vertexData.data(), vertexData.size() * sizeof(VertexData));
	}
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

size_t D3D12GraphicEngine::GetResourceArraySize(DirectXResourceHandle handle) {
	size_t stride = resourceContainer_->GetResourceStrideInBytes(handle);
	if (stride == 0) {
		QFE_LOG("Resource stride is zero for handle: " + std::to_string(static_cast<uint32_t>(handle)));
		return 0;
	}
	size_t size = resourceContainer_->GetResourceSizeInBytes(handle);
	return size / stride;
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
	if(rootParameterTypes.size() != rootResources.size()) {
		// PSOのルートパラメータの数と渡されたリソースの数が異なる場合はエラー
		assert(false);
		return;
	}

	for(int i = 0; i < rootParameterTypes.size(); ++i) {
		D3D12_ROOT_PARAMETER_TYPE rootParameterType = rootParameterTypes[i];

		if(rootParameterType == D3D12_ROOT_PARAMETER_TYPE_CBV) {
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
	depthResourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthResourceDesc.SampleDesc.Count = 1;
	depthResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	D3D12_HEAP_PROPERTIES depthHeapProps{};
	depthHeapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
	D3D12_CLEAR_VALUE depthClearValue{};
	depthClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthClearValue.DepthStencil.Depth = 1.0f;

	DirectXResourceHandle handle = resourceContainer_->CreateResource(
		directXDevice_->GetDevice(),
		depthResourceDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		D3D12_HEAP_TYPE_DEFAULT,
		&depthClearValue);
	
	// DSVの生成
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	CereateViewInfo createViewInfo{};
	createViewInfo.viewType = ViewTypeFlags::DepthStencilView;
	createViewInfo.dsvDesc = dsvDesc;
	resourceContainer_->CreateResourceView(handle, createViewInfo);

	return handle;
}

void D3D12GraphicEngine::ClearDepthStencil(DirectXResourceHandle depthStencilHandle) {
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = resourceContainer_->GetDescriptorHandleCPU(depthStencilHandle, ViewTypeFlags::DepthStencilView);
	commandManager_->GetCommandList(D3D12_COMMAND_LIST_TYPE_DIRECT)->ClearDepthStencilView(
		dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
}