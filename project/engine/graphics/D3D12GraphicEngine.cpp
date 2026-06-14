#include "D3D12GraphicEngine.h"
#include "EngineDefines.h"

#include "common/AssimpModelLoader.h"

#include "dx12/checker/DirectX12DebugCore.h"
#include "dx12/DirectXDevice.h"
#include "dx12/DirectXResourceContainer.h"
#include "dx12/descriptors/DescriptorHeapManager.h"
#include "dx12/command/DirectXCommandManager.h"
#include "dx12/RenderPass.h"
#include "dx12/Fence.h"

#include "dx12/pipeline/pso/ShaderCompiler.h"

#include "dx12/pipeline/GraphicPipelineManager.h"
#include "dx12/TextureLoader.h"
#include "dx12/ModelDataContainer.h"
#include "dx12/VertexBufferContainer.h"

#include "dx12/descriptors/Data/DescriptorHandles.h"

using namespace QFE::GRAPHIC;

namespace {
	float clearColor[4] = { 0.1f, 0.25f, 0.5f, 1.0f };
	const uint32_t kInvalidTextureHandle = UINT32_MAX;
}

QFE::GRAPHIC::D3D12GraphicEngine::D3D12GraphicEngine(HWND hwnd0) :
	hwnd_(hwnd0),
	debugCore_(std::make_unique<INTERNAL::DirectX12DebugCore>()),
	directXDevice_(std::make_unique<INTERNAL::DirectXDevice>()),
	resourceContainer_(std::make_unique<INTERNAL::DirectXResourceContainer>()),
	descriptorHeapManager_(std::make_unique<INTERNAL::DescriptorHeapManager>()),
	commandManager_(std::make_unique<INTERNAL::DirectXCommandManager>()),
	renderPass_(std::make_unique<INTERNAL::RenderPass>()),
	fence_(std::make_unique<INTERNAL::Fence>()),
	graphicPipelineManager_(std::make_unique<INTERNAL::GraphicPipelineManager>()),
	textureLoader_(std::make_unique<INTERNAL::TextureLoader>()),
	modelDataContainer_(std::make_unique<INTERNAL::ModelDataContainer>()),
	vertexBufferContainer_(std::make_unique<INTERNAL::VertexBufferContainer>()),
	shaderCompiler_(std::make_unique<INTERNAL::ShaderCompiler>())

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
	INTERNAL::DirectXResourceContainerInitializeInfo resourceContainerInfo{};
	resourceContainerInfo.assignRtvFunc = [&](ID3D12Resource* resource, const D3D12_RENDER_TARGET_VIEW_DESC* desc) 
		{return descriptorHeapManager_->AssignRtvHeap(resource, desc); };
	resourceContainerInfo.assignSrvFunc = [&](ID3D12Resource* resource, const D3D12_SHADER_RESOURCE_VIEW_DESC* desc)
		{return descriptorHeapManager_->AssignSrvHeap(resource, *desc); };
	resourceContainerInfo.assignDsvFunc = [&](ID3D12Resource* resource, const D3D12_DEPTH_STENCIL_VIEW_DESC* desc)
		{return descriptorHeapManager_->AssignDsvHeap(resource, desc); };
	resourceContainerInfo.assignUavFunc = [&](ID3D12Resource* resource, const D3D12_UNORDERED_ACCESS_VIEW_DESC* desc)
		{return INTERNAL::DescriptorHandles{}; };
	resourceContainer_->Initialize(resourceContainerInfo);

	// SwapChainの初期化
	INTERNAL::RenderPassInitializeInfo renderPassInfo{};
	renderPassInfo.width = width;
	renderPassInfo.height = height;
	renderPassInfo.hwnd = hwnd_;
	renderPassInfo.device = directXDevice_->GetDevice();
	renderPassInfo.dxgiFactory = directXDevice_->GetDxgiFactory();
	renderPassInfo.commandQueue = commandManager_->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
	renderPassInfo.assignRtvFunc = [&](ID3D12Resource* resource, const D3D12_RENDER_TARGET_VIEW_DESC* desc) 
		{return descriptorHeapManager_->AssignRtvHeap(resource, desc);};
	renderPassInfo.assignSrvFunc = [&](ID3D12Resource* resource, const D3D12_SHADER_RESOURCE_VIEW_DESC* desc) 
		{return descriptorHeapManager_->AssignSrvHeap(resource, *desc); };

	renderPass_->Initialize(renderPassInfo);
	// Fenceの初期化
	fence_->Initialize(directXDevice_->GetDevice());

	// DirectXCommonの名残.フェンスの初期化以降の処理.
	LegacyInitialize(width, height);

	// グラフィックパイプラインマネージャの初期化
	graphicPipelineManager_->Initialize(
		[&](const std::wstring& filePath, const wchar_t* profile) {return shaderCompiler_->CompileShader(filePath, profile); },
		directXDevice_->GetDevice());
	
	// テクスチャ管理クラスの初期化
	INTERNAL::TextureLoaderInitializeInfo textureLoaderInfo{};
	// テクスチャのリソース作成やSRV作成、データアップロードに必要な情報をTextureLoaderInitializeInfo構造体にセット
	textureLoaderInfo.device = directXDevice_->GetDevice();
	// テクスチャのリソース作成に必要な関数をDirectXResourceContainerのCreateResource関数を呼び出すラムダ式で初期化
	textureLoaderInfo.createResourceFunc = 
		[&](const D3D12_RESOURCE_DESC& desc) { return resourceContainer_->CreateResource(desc, D3D12_RESOURCE_STATE_GENERIC_READ); };
	// テクスチャのSRV作成に必要な関数をDirectXResourceContainerのCreateResourceView関数を呼び出すラムダ式で初期化
	textureLoaderInfo.createShaderResourceViewFunc = 
		[&](INTERNAL::DirectXResourceHandle resourceHandle, const D3D12_SHADER_RESOURCE_VIEW_DESC& desc) {
		INTERNAL::CereateViewInfo createViewInfo{};
		createViewInfo.viewType = INTERNAL::ViewTypeFlags::ShaderResourceView;
		createViewInfo.dsvDesc = {};
		createViewInfo.rtvDesc = {};
		createViewInfo.srvDesc = desc;
		createViewInfo.uavDesc = {};
		resourceContainer_->CreateResourceView(resourceHandle, createViewInfo);};
	// テクスチャのアップロードに必要な関数をDirectXResourceContainerのUploadResource関数を呼び出すラムダ式で初期化
	textureLoaderInfo.uploadTextureDataFunc = 
		[&](INTERNAL::DirectXResourceHandle resourceHandle, const std::vector<D3D12_SUBRESOURCE_DATA>& subresources) {
		resourceContainer_->UploadResource(resourceHandle, subresources, directXDevice_->GetDevice(), commandManager_->GetCommandList(D3D12_COMMAND_LIST_TYPE_DIRECT));
		};
	// テクスチャローダーを初期化
	textureLoader_->Initialize(textureLoaderInfo);

	// モデル頂点リソース管理クラスの初期化
	modelDataContainer_->Initialize();
	vertexBufferContainer_->Initialize();
}

void D3D12GraphicEngine::PreDraw() {
	// スワップチェーンのリソース状態を描画可能に変更
	renderPass_->PreDraw(commandManager_->GetCommandList(D3D12_COMMAND_LIST_TYPE_DIRECT));

	// デプスステンシルのクリア
	ClearDepthStencil();
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

	// テクスチャのアップロードに使用した中間リソースの解放
	textureLoader_->ReleaseIntermediateResources();
}

void D3D12GraphicEngine::Shutdown() {
	modelDataContainer_->Finalize();
	vertexBufferContainer_->Finalize();
	textureLoader_->Finalize();
	fence_->Shutdown();
	shaderCompiler_->Finalize();
	directXDevice_->Shutdown();
}

TextureHandle D3D12GraphicEngine::LoadTexture(const std::string& filePath) {
	return static_cast<TextureHandle>(textureLoader_->LoadTexture(filePath));
}

VertexBufferHandle D3D12GraphicEngine::LoadMesh(const std::vector<VertexData>& vertexData, const std::string& meshName) {
	return static_cast<VertexBufferHandle>(vertexBufferContainer_->Assign(directXDevice_->GetDevice(), vertexData, meshName));
}

std::vector<VertexBufferHandle> D3D12GraphicEngine::LoadMeshesFromModel(ModelHandle modelHandle) {
	const auto& modelData = modelDataContainer_->GetModelData(static_cast<uint32_t>(modelHandle));
	std::vector<VertexBufferHandle> meshHandles;
	for (size_t i = 0; i < modelData.meshes.size(); ++i) {
		const auto& meshData = modelData.meshes[i];
		std::string meshName = modelData.name + "_mesh_" + std::to_string(i);
		VertexBufferHandle meshHandle = LoadMesh(meshData.vertices.GetInternalVector(), meshName);
		meshHandles.push_back(meshHandle);
	}
	return meshHandles;
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

void D3D12GraphicEngine::LegacyInitialize(uint32_t width, uint32_t height) {
	CreateDepthStencilBuffer(width, height);
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

void QFE::GRAPHIC::D3D12GraphicEngine::ClearDepthStencil() {
	commandManager_->GetCommandList(D3D12_COMMAND_LIST_TYPE_DIRECT)->ClearDepthStencilView(
		dsvHandle_.cpuHandle_, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
}
