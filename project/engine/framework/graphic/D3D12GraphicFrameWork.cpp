#include "D3D12GraphicFrameWork.h"

#include "graphics/D3D12GraphicEngine.h"
#include "graphics/dx12/pipeline/GraphicPipelineManager.h"
#include "graphics/dx12/pipeline/RaytracingPipelineManager.h"
#include "graphics/dx12/pipeline/ComputePipelineManager.h"
#include "graphics/dx12/vram/DirectXResourceAllocator.h"
#include "graphics/dx12/vram/resources/DirectXResourceContainer.h"
#include "graphics/dx12/TextureLoader.h"
#include "graphics/dx12/DirectXDevice.h"
#include "graphics/dx12/command/DirectXCommandManager.h"
#include "graphics/dx12/RenderPass.h"
#include "graphics/dx12/pipeline/rtpso/RaytracingAccelerationStructure.h"
#include "graphics/dx12/Fence.h"

#include "graphics/dx12/pipeline/rtpso/TLAS.h"

#include "core/math/transform/Transform.h"
#include "../../resources/Shaders/ShaderStructs/hlslTypeToCpp.h"

std::unique_ptr<QFE::GRAPHIC::D3D12GraphicEngine> QFE::FRAMEWORK::CreateGraphicEngine(HWND hwnd) {
	// ウィンドウのハンドルを取得してグラフィックエンジンを初期化
	std::unique_ptr<QFE::GRAPHIC::D3D12GraphicEngine> graphicEngine =
		std::make_unique<QFE::GRAPHIC::D3D12GraphicEngine>(hwnd);
	graphicEngine->Initialize();
	return graphicEngine;
}

bool QFE::FRAMEWORK::PreDrawGraphicEngine(QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine) {
	if(graphicEngine == nullptr) {
		QFE_LOG("graphicEngine is null");
		return false;
	}
	graphicEngine->PreDraw();
	return true;
}

bool QFE::FRAMEWORK::PostDrawGraphicEngine(QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine) {
	if(graphicEngine == nullptr) {
		QFE_LOG("graphicEngine is null");
		return false;
	}
	graphicEngine->PostDraw();
	return true;
}

bool QFE::FRAMEWORK::ShutdownGraphicEngine(QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine) {
	if(graphicEngine == nullptr) {
		QFE_LOG("graphicEngine is null");
		return false;
	}
	graphicEngine->Shutdown();
	return true;
}

bool QFE::FRAMEWORK::GetWhite1x1TextureHandle(
	QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine, QFE::GRAPHIC::DirectXResourceHandle& outTextureHandle) {
	
	QFE::GRAPHIC::TextureLoader* textureLoader = graphicEngine->GetTextureLoader();
	outTextureHandle = textureLoader->GetDummyWhite1x1TextureHandle();
	// 成否の確認
	if(outTextureHandle == QFE::GRAPHIC::DirectXResourceHandle::Invalid) {
		QFE_LOG("Failed to get white 1x1 texture handle.");
		return false;
	}
	return true;
}

bool QFE::FRAMEWORK::GetBlackCubeMapTextureHandle(
	QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine, QFE::GRAPHIC::DirectXResourceHandle& outTextureHandle) {

	QFE::GRAPHIC::TextureLoader* textureLoader = graphicEngine->GetTextureLoader();
	outTextureHandle = textureLoader->GetDummyBlackCubeMapHandle();
	// 成否の確認
	if (outTextureHandle == QFE::GRAPHIC::DirectXResourceHandle::Invalid) {
		QFE_LOG("Failed to get black cube map texture handle.");
		return false;
	}
	return true;
}

bool QFE::FRAMEWORK::GetResourceArraySize(
	QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine, QFE::GRAPHIC::DirectXResourceHandle resourceHandle, size_t& outResourceArraySize) {

	QFE::GRAPHIC::DirectXResourceContainer* resourceContainer = graphicEngine->GetDirectXResourceContainer();

	// リソースの配列サイズを取得
	size_t strideSize = resourceContainer->GetResourceStrideInBytes(resourceHandle);
	size_t resourceSize = resourceContainer->GetResourceSizeInBytes(resourceHandle);
	
	if (strideSize == 0) {
		QFE_LOG("Failed to get resource stride size.");
		return false;
	}

	outResourceArraySize = resourceSize / strideSize;
	return true;
}

bool QFE::FRAMEWORK::GetRenderResourceHandle(
	QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine, QFE::GRAPHIC::RenderTargetHandle renderTargetHandle,QFE::GRAPHIC::DirectXResourceHandle& outResourceHandle) {
	
	QFE::GRAPHIC::RenderPass* renderPass = graphicEngine->GetRenderPass();
	outResourceHandle = renderPass->GetRenderTargetResourceHandle(renderTargetHandle);
	// 成否の確認
	if (outResourceHandle == QFE::GRAPHIC::DirectXResourceHandle::Invalid) {
		QFE_LOG("Failed to get render resource handle.");
		return false;
	}
	return true;
}

bool QFE::FRAMEWORK::GetDepthStencilResourceHandle(
	QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine, QFE::GRAPHIC::DirectXResourceHandle& outDepthStencilHandle) {

	QFE::GRAPHIC::DirectXResourceHandle depthStencilHandle = graphicEngine->GetDepthStencilBufferHandle();
	outDepthStencilHandle = depthStencilHandle;
	// 成否の確認
	if (outDepthStencilHandle == QFE::GRAPHIC::DirectXResourceHandle::Invalid) {
		QFE_LOG("Failed to get depth stencil resource handle.");
		return false;
	}
	return true;
}

bool QFE::FRAMEWORK::CreateShaderPair(
	QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine, const std::string& vsDirName, const std::string& psDirName, const std::string& vsFileName, const std::string& psFileName, QFE::GRAPHIC::ShaderPairHandle& outShaderPairHandle) {
	// シェーダーペア生成の設定
	QFE::GRAPHIC::ShaderPairElement shaderPairElement;
	shaderPairElement.vsDirName = vsDirName;
	shaderPairElement.psDirName = psDirName;
	shaderPairElement.vsFileName = vsFileName;
	shaderPairElement.psFileName = psFileName;

	// シェーダーペアを生成
	QFE::GRAPHIC::GraphicPipelineManager* pipelineManager = graphicEngine->GetGraphicPipelineManager();
	QFE::GRAPHIC::ShaderPairHandle shaderPairHandle = pipelineManager->GenerateShaderPair(shaderPairElement);
	outShaderPairHandle = shaderPairHandle;

	// シェーダーペアの生成に成功したかを返す
	return outShaderPairHandle != QFE::GRAPHIC::ShaderPairHandle::Invalid;
}

bool QFE::FRAMEWORK::CreateGraphicPSO(
	QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine, const QFE::GRAPHIC::ShaderPairHandle& shaderPairHandle,
	const QFE::GRAPHIC::RasterizerType& rasterizerType, const QFE::GRAPHIC::BlendMode& blendMode,
	const QFE::GRAPHIC::DepthStencilDescType& depthStencilDescType, QFE::GRAPHIC::PSOHandle& outPSOHandle) {

	// 使用するパイプラインマネージャを取得
	QFE::GRAPHIC::GraphicPipelineManager* pipelineManager = graphicEngine->GetGraphicPipelineManager();
	// パイプラインステートオブジェクトを生成
	QFE::GRAPHIC::PSOHandle psoHandle = pipelineManager->GeneratePipelineStateObject(
		graphicEngine->GetDirectXDevice()->GetDevice(),
		shaderPairHandle,
		blendMode,
		rasterizerType,
		depthStencilDescType
	);
	// 生成したPSOハンドルを出力引数に設定
	outPSOHandle = psoHandle;
	if(psoHandle == QFE::GRAPHIC::PSOHandle::Invalid) {
		QFE_LOG("Failed to create PSO.");
		return false;
	}
	return true;
}

bool QFE::FRAMEWORK::GetGraphicPSORootParameterTypeList(
	QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine, const QFE::GRAPHIC::PSOHandle& psoHandle, 
	std::vector<D3D12_ROOT_PARAMETER_TYPE>& outRootParameterTypeList) {


	// 使用するパイプラインマネージャを取得
	QFE::GRAPHIC::GraphicPipelineManager* pipelineManager = graphicEngine->GetGraphicPipelineManager();

	// PSOハンドルからルートパラメータのタイプを取得
	outRootParameterTypeList = pipelineManager->GetRootParameterTypes(psoHandle);
	// 成否の確認
	if(outRootParameterTypeList.empty()) {
		QFE_LOG("Failed to get root parameter type list.");
		return false;
	}
	return true;
}

bool QFE::FRAMEWORK::CreateRayTracingPSO(
	QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine, QFE::GRAPHIC::RTPSOHandle& outPSOHandle,
	const std::string& dirPath, const std::string& rgsFileName) {

	// 使用するレイトレーシングパイプラインマネージャを取得
	QFE::GRAPHIC::RaytracingPipelineManager* rayTracingPipelineManager_ = graphicEngine->GetRayTracingPipelineManager();
	// レイトレーシングパイプラインステートオブジェクトを生成
	outPSOHandle = rayTracingPipelineManager_->CreateRaytracingPipelineStateObject(ConvertString(dirPath + rgsFileName), L"lib_6_3");
	// 成否の確認
	if (outPSOHandle == QFE::GRAPHIC::RTPSOHandle::Invalid) {
		QFE_LOG("Failed to create ray tracing PSO.");
		return false;
	}
	return true;
}

bool QFE::FRAMEWORK::CreateOffScreenRenderTarget(
	QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine,
	QFE::GRAPHIC::RenderTargetHandle& outRenderTargetHandle,
	uint32_t width, uint32_t height, DXGI_FORMAT format) {

	// 使用するレンダーパスを取得
	QFE::GRAPHIC::RenderPass* renderPass = graphicEngine->GetRenderPass();
	outRenderTargetHandle = renderPass->CreateOffscreenRenderTarget(width, height, format);
	// 成否の確認
	if(outRenderTargetHandle == QFE::GRAPHIC::RenderTargetHandle::Invalid) {
		QFE_LOG("Failed to create offscreen render target.");
		return false;
	}
	return true;
}

bool QFE::FRAMEWORK::CreateViewport(
	QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine, QFE::GRAPHIC::ViewPortHandle& outViewportHandle, uint32_t width, uint32_t height) {
	
	// ビューポートコンテナを取得
	QFE::UniqueContainer<D3D12_VIEWPORT>& viewports_ = graphicEngine->GetViewports();
	// ビューポートを作成
	D3D12_VIEWPORT viewport{};
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	viewport.Width = static_cast<float>(width);
	viewport.Height = static_cast<float>(height);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	// ビューポートをコンテナに追加し、ハンドルを取得
	uint32_t handle = viewports_.Add(std::to_string(width) + "x" + std::to_string(height), viewport);
	outViewportHandle = static_cast<QFE::GRAPHIC::ViewPortHandle>(handle);
	// 成否の確認
	if (outViewportHandle == QFE::GRAPHIC::ViewPortHandle::Invalid) {
		QFE_LOG("Failed to create viewport.");
		return false;
	}
	return true;
}

bool QFE::FRAMEWORK::CreateScissorRect(
	QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine, QFE::GRAPHIC::ScissorRectHandle& outScissorRectHandle,
	int32_t left, int32_t top, int32_t right, int32_t bottom) {

	// シザリング矩形コンテナを取得
	QFE::UniqueContainer<D3D12_RECT>& scissorRects_ = graphicEngine->GetScissorRects();
	// シザリング矩形を作成
	D3D12_RECT scissorRect{};
	scissorRect.left = left;
	scissorRect.top = top;
	scissorRect.right = right;
	scissorRect.bottom = bottom;
	// シザリング矩形をコンテナに追加し、ハンドルを取得
	uint32_t handle = scissorRects_.Add(std::to_string(left) + "_" + std::to_string(top) + "_" + std::to_string(right) + "_" + std::to_string(bottom), scissorRect);
	outScissorRectHandle = static_cast<QFE::GRAPHIC::ScissorRectHandle>(handle);
	// 成否の確認
	if (outScissorRectHandle == QFE::GRAPHIC::ScissorRectHandle::Invalid) {
		QFE_LOG("Failed to create scissor rect.");
		return false;
	}
	return true;
}

bool QFE::FRAMEWORK::CreateUAVBuffer(
	QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine, QFE::GRAPHIC::DirectXResourceHandle& outUAVBufferHandle,
	uint32_t width, uint32_t height, const std::wstring& name) {

	QFE::GRAPHIC::DirectXDevice* directXDevice = graphicEngine->GetDirectXDevice();
	QFE::GRAPHIC::DirectXResourceContainer* resourceContainer = graphicEngine->GetDirectXResourceContainer();

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
	QFE::GRAPHIC::DirectXResourceHandle handle =
		resourceContainer->CreateResource(
			directXDevice->GetDevice(), texDesc, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_HEAP_TYPE_DEFAULT);

	// UAVの生成
	QFE::GRAPHIC::CreateViewInfo createViewInfo{};
	createViewInfo.viewType = QFE::GRAPHIC::ViewTypeFlags::UnorderedAccessView;
	createViewInfo.uavDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	createViewInfo.uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	// Viewの生成とリソース名の設定
	resourceContainer->CreateResourceView(handle, createViewInfo);
	resourceContainer->SetResourceName(handle, name);
	// 成否の確認
	outUAVBufferHandle = handle;
	if(outUAVBufferHandle == QFE::GRAPHIC::DirectXResourceHandle::Invalid) {
		QFE_LOG("Failed to create UAV buffer.");
		return false;
	}
	return true;
}

bool QFE::FRAMEWORK::CreateObject3dGBufferRootResources(
	QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine, QFE::GRAPHIC::DirectXResourceHandle* rootResources, size_t rootResourcesSize) {

	// ルートリソースの数を確認
	if (rootResourcesSize < 3) {
		QFE_LOG("rootResourcesSize is too small. It should be at least 3.");
		return false;
	}

	// リソースアロケータを取得
	QFE::GRAPHIC::DirectXResourceAllocator* resourceAllocator = graphicEngine->GetDirectXResourceAllocator();
	// テクスチャマネージャを取得
	QFE::GRAPHIC::TextureLoader* textureLoader = graphicEngine->GetTextureLoader();

	// EulerTransform
	TransformationMatrix transformMatrix;
	transformMatrix.World = QFE::MATH::Matrix4x4::MakeIndentity4x4();
	transformMatrix.WVP = QFE::MATH::Matrix4x4::MakeIndentity4x4();
	QFE::GRAPHIC::DirectXResourceHandle transformMatrixBufferHandle =
		resourceAllocator->AllocateConstantBuffer<TransformationMatrix>("TransformMatrixBuffer");
	// Material
	Material material;
	material.color = { 1.0f, 1.0f, 1.0f, 1.0f };
	QFE::GRAPHIC::DirectXResourceHandle materialBufferHandle =
		resourceAllocator->AllocateConstantBuffer<Material>("MaterialBuffer");
	
	//Texture
	QFE::GRAPHIC::DirectXResourceHandle textureHandle = textureLoader->GetDummyWhite1x1TextureHandle();

	// ルートリソースに設定
	rootResources[0] = transformMatrixBufferHandle;
	rootResources[1] = materialBufferHandle;
	rootResources[2] = textureHandle;

	return true;
}

bool QFE::FRAMEWORK::TransitionResourceToState(
	QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine, 
	const QFE::GRAPHIC::DirectXResourceHandle& resourceHandle,
	D3D12_RESOURCE_STATES newState) {

	QFE::GRAPHIC::DirectXCommandManager* commandManager = graphicEngine->GetDirectXCommandManager();
	QFE::GRAPHIC::DirectXResourceContainer* resourceContainer = graphicEngine->GetDirectXResourceContainer();

	// コマンドリストを取得
	ID3D12GraphicsCommandList* commandList = commandManager->GetCommandList(D3D12_COMMAND_LIST_TYPE_DIRECT);
	return resourceContainer->TransitionResource(resourceHandle, commandList, newState);
}

bool QFE::FRAMEWORK::CreateCameraPosBuffer(
	QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine, const std::wstring& name, QFE::GRAPHIC::DirectXResourceHandle& outCameraPosBufferHandle) {

	// リソースアロケータを取得
	QFE::GRAPHIC::DirectXResourceAllocator* resourceAllocator = graphicEngine->GetDirectXResourceAllocator();
	// カメラ位置バッファを作成
	outCameraPosBufferHandle = resourceAllocator->AllocateConstantBuffer<QFE::MATH::Vector3>(ConvertString(name));
	// 成否の確認
	if(outCameraPosBufferHandle == QFE::GRAPHIC::DirectXResourceHandle::Invalid) {
		QFE_LOG("Failed to create camera position buffer.");
		return false;
	}
	return true;
}

void QFE::FRAMEWORK::UpdateObject3dWVPMatrix(
	QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine, const QFE::GRAPHIC::DirectXResourceHandle& transformMatrixBufferHandle, 
	const QFE::MATH::EulerTransform& transform, const QFE::MATH::Matrix4x4& viewProjectionMatrix) {

	TransformationMatrix* transformMatrixData =
		graphicEngine->GetConstantBufferData<TransformationMatrix>(transformMatrixBufferHandle);
	transformMatrixData->World = QFE::MATH::Matrix4x4::MakeAffineMatrix(transform);
	transformMatrixData->WVP = QFE::MATH::Matrix4x4::Multiply(transformMatrixData->World, viewProjectionMatrix);
}

std::vector<QFE::MATH::Vector3> QFE::FRAMEWORK::GetModelVertexPositions(const VertexData* vertices, size_t vertexCount) {
	std::vector<QFE::MATH::Vector3> vertexPositions;
	vertexPositions.reserve(vertexCount);
	for (size_t i = 0; i < vertexCount; ++i) {
		const VertexData& vertex = vertices[i];
		vertexPositions.push_back({ vertex.position.x, vertex.position.y, vertex.position.z });
	}
	return vertexPositions;
}

bool QFE::FRAMEWORK::CreateVertexBuffer(
	QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine, const std::vector<VertexData>& vertexPositions,
	const std::string& meshName, QFE::GRAPHIC::DirectXResourceHandle& outVertexBufferHandle) {

	// 使用機能の取得
	QFE::GRAPHIC::DirectXDevice* directXDevice = graphicEngine->GetDirectXDevice();
	QFE::GRAPHIC::DirectXResourceAllocator* resourceAllocator = graphicEngine->GetDirectXResourceAllocator();
	QFE::GRAPHIC::DirectXResourceContainer* resourceContainer = graphicEngine->GetDirectXResourceContainer();

	// バッファを作成
	QFE::GRAPHIC::DirectXResourceHandle handle =
		resourceContainer->CreateBuffer(directXDevice->GetDevice(), vertexPositions.size() * sizeof(VertexData));
	QFE_LOG("VertexBuffer created for mesh: " + meshName + ", size: " + std::to_string(vertexPositions.size() * sizeof(VertexData)) + " bytes");
	// バッファのストライドを設定
	resourceContainer->SetResourceStrideInBytes(handle, sizeof(VertexData));
	// バッファにデータをコピー
	resourceContainer->MapResource(handle);
	VertexData* mappedData = resourceContainer->GetMappedData<VertexData>(handle);
	if (mappedData) {
		memcpy(mappedData, vertexPositions.data(), vertexPositions.size() * sizeof(VertexData));
	}
	resourceContainer->SetResourceName(handle, ConvertString(meshName));
	resourceContainer->SetResourceStrideInBytes(handle, sizeof(VertexData));

	// 成否の確認
	outVertexBufferHandle = handle;
	if(outVertexBufferHandle == QFE::GRAPHIC::DirectXResourceHandle::Invalid) {
		QFE_LOG("Failed to create vertex buffer for mesh: " + meshName);
		return false;
	}
	return true;
}

bool QFE::FRAMEWORK::CreateIndexBuffer(QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine, const std::vector<uint32_t>& indices, const std::string& meshName, QFE::GRAPHIC::DirectXResourceHandle& outIndexBufferHandle) {
	// 使用機能の取得
	QFE::GRAPHIC::DirectXDevice* directXDevice = graphicEngine->GetDirectXDevice();
	QFE::GRAPHIC::DirectXResourceContainer* resourceContainer = graphicEngine->GetDirectXResourceContainer();

	// バッファを作成（サイズはインデックス数 * 4 バイト）
	QFE::GRAPHIC::DirectXResourceHandle handle =
		resourceContainer->CreateBuffer(directXDevice->GetDevice(), indices.size() * sizeof(uint32_t));
	QFE_LOG("IndexBuffer created for mesh: " + meshName + ", size: " + std::to_string(indices.size() * sizeof(uint32_t)) + " bytes");

	// インデックスバッファのストライドを設定（uint32_t）
	resourceContainer->SetResourceStrideInBytes(handle, sizeof(uint32_t));

	// バッファにデータをコピー
	resourceContainer->MapResource(handle);
	uint32_t* mappedData = resourceContainer->GetMappedData<uint32_t>(handle);
	if (mappedData) {
		memcpy(mappedData, indices.data(), indices.size() * sizeof(uint32_t));
	}
	resourceContainer->SetResourceName(handle, ConvertString(meshName + "_idx"));
	// 再設定（安全のため）
	resourceContainer->SetResourceStrideInBytes(handle, sizeof(uint32_t));

	// 出力ハンドル設定と成否判定
	outIndexBufferHandle = handle;
	if (outIndexBufferHandle == QFE::GRAPHIC::DirectXResourceHandle::Invalid) {
		QFE_LOG("Failed to create index buffer for mesh: " + meshName);
		return false;
	}
	return true;
}

bool QFE::FRAMEWORK::CreateBLAS(
	QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine, const std::vector<VertexData>& vertices,
	const std::vector<uint32_t>& indices,
	const std::string& name, QFE::GRAPHIC::BLASHandle& outBLASHandle) {

	// 使用機能の取得
	QFE::GRAPHIC::DirectXDevice* directXDevice = graphicEngine->GetDirectXDevice();	
	QFE::GRAPHIC::RaytracingAccelerationStructure* accelerationStructure = graphicEngine->GetRaytracingAccelerationStructure();

	// vertexをVector3に変換してBLASを作成
	std::vector<QFE::MATH::Vector3> vertexPositions = GetModelVertexPositions(vertices.data(), vertices.size());
	outBLASHandle = accelerationStructure->CreateBLAS(
		directXDevice->GetDevice5(),
		graphicEngine->GetDirectXCommandManager()->GetCommandList4(D3D12_COMMAND_LIST_TYPE_DIRECT),
		vertexPositions, indices, name);
	// BLASの作成に失敗した場合はログを出力してfalseを返す
	if(outBLASHandle == QFE::GRAPHIC::BLASHandle::Invalid) {
		QFE_LOG("Failed to create BLAS for model: " + name);
		return false;
	}
	return true;
}

bool QFE::FRAMEWORK::UpdateBLASInstanceBuffer(
	QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine, const std::vector<std::pair<QFE::GRAPHIC::BLASHandle, QFE::MATH::Matrix4x4>>& instances) {

	// 使用機能の取得
	QFE::GRAPHIC::DirectXCommandManager* commandManager = graphicEngine->GetDirectXCommandManager();
	QFE::GRAPHIC::Fence* fence = graphicEngine->GetFence();
	QFE::GRAPHIC::RaytracingAccelerationStructure* accelerationStructure = graphicEngine->GetRaytracingAccelerationStructure();

	// インスタンスを更新するために構造体に変換
	std::vector<QFE::GRAPHIC::RaytracingInstance> raytracingInstances;
	for (const auto& instance : instances) {
		QFE::GRAPHIC::RaytracingInstance raytracingInstance;
		raytracingInstance.blasHandle = instance.first;
		raytracingInstance.worldMatrix = instance.second;
		raytracingInstances.push_back(raytracingInstance);
	}

	// TLASの更新
	accelerationStructure->UpdateTLAS(commandManager->GetCommandList4(D3D12_COMMAND_LIST_TYPE_DIRECT), raytracingInstances);
	commandManager->ExecuteCommandList();
	fence->Signal(commandManager->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT));
	fence->Wait();
	commandManager->ResetCommandList();
	return true;
}

bool QFE::FRAMEWORK::SetRenderTarget(
	QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine, const QFE::GRAPHIC::DirectXResourceHandle& depthStencilBufferHandle,
	const std::vector<QFE::GRAPHIC::RenderTargetHandle>& renderTargets) {

	// 使用機能の取得
	QFE::GRAPHIC::DirectXCommandManager* commandManager = graphicEngine->GetDirectXCommandManager();
	QFE::GRAPHIC::RenderPass* renderPass = graphicEngine->GetRenderPass();

	// レンダーターゲットを設定
	// TODO: ここでレンダーターゲットを設定する際に、レンダーターゲットの数が0の場合は深度ステンシルバッファのみを設定するようにする必要がある
	// TODO: 成否の確認を行う必要がある
	renderPass->SetRenderTarget(
		commandManager->GetCommandList(D3D12_COMMAND_LIST_TYPE_DIRECT), depthStencilBufferHandle, renderTargets);

	return true;
}

bool QFE::FRAMEWORK::DrawGraphicPSO(
	QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine, const QFE::GRAPHIC::PSOHandle& psoHandle,
	const QFE::GRAPHIC::ViewPortHandle& viewportHandle, const QFE::GRAPHIC::ScissorRectHandle& scissorRectHandle,
	const QFE::GRAPHIC::DirectXResourceHandle& vertexBufferHandle, const std::vector<QFE::GRAPHIC::DirectXResourceHandle>& rootResources,
	const std::vector<QFE::GRAPHIC::RenderTargetHandle>& renderTargets) {

	// 使用機能の取得
	QFE::GRAPHIC::DirectXCommandManager* commandManager = graphicEngine->GetDirectXCommandManager();
	QFE::GRAPHIC::RenderPass* renderPass = graphicEngine->GetRenderPass();
	QFE::GRAPHIC::DirectXResourceContainer* resourceContainer = graphicEngine->GetDirectXResourceContainer();
	QFE::GRAPHIC::GraphicPipelineManager* graphicPipelineManager = graphicEngine->GetGraphicPipelineManager();
	QFE::UniqueContainer<D3D12_VIEWPORT>& viewports = graphicEngine->GetViewports();
	QFE::UniqueContainer<D3D12_RECT>& scissorRects = graphicEngine->GetScissorRects();

	QFE::GRAPHIC::DirectXResourceHandle depthStencilBufferHandle = graphicEngine->GetDepthStencilBufferHandle();

	ID3D12GraphicsCommandList* commandList = commandManager->GetCommandList(D3D12_COMMAND_LIST_TYPE_DIRECT);

	renderPass->SetRenderTarget(
		commandList, depthStencilBufferHandle, renderTargets);

	commandList->RSSetViewports(1, viewports.GetData(static_cast<uint32_t>(viewportHandle)));
	commandList->RSSetScissorRects(1, scissorRects.GetData(static_cast<uint32_t>(scissorRectHandle)));

	commandList->SetPipelineState(graphicPipelineManager->GetPipelineState(psoHandle));
	commandList->SetGraphicsRootSignature(graphicPipelineManager->GetRootSignature(psoHandle));

	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView = resourceContainer->GetVertexBufferView(vertexBufferHandle);
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView);

	std::vector<D3D12_ROOT_PARAMETER_TYPE> rootParameterTypes = graphicPipelineManager->GetRootParameterTypes(psoHandle);
	if (rootParameterTypes.size() != rootResources.size()) {
		// PSOのルートパラメータの数と渡されたリソースの数が異なる場合はエラー
		assert(false);
		return false;
	}

	for (int i = 0; i < rootParameterTypes.size(); ++i) {
		D3D12_ROOT_PARAMETER_TYPE rootParameterType = rootParameterTypes[i];

		if (rootParameterType == D3D12_ROOT_PARAMETER_TYPE_CBV) {
			D3D12_GPU_VIRTUAL_ADDRESS gpuHandle = resourceContainer->GetGpuVirtualAddress(rootResources[i]);
			commandList->SetGraphicsRootConstantBufferView(static_cast<UINT>(i), gpuHandle);
		} else {
			D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = resourceContainer->GetDescriptorHandleGPU(rootResources[i], rootParameterType);
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

	size_t vertexCount = 0;
	if(!GetResourceArraySize(graphicEngine, vertexBufferHandle, vertexCount)) {
		assert(false);
		return false;
	}
	UINT vertexCountUINT = static_cast<UINT>(vertexCount);
	commandList->DrawInstanced(vertexCountUINT, 1, 0, 0);
	return true;
}

bool QFE::FRAMEWORK::DrawGraphicPSO(
	QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine, const QFE::GRAPHIC::PSOHandle& psoHandle, 
	const QFE::GRAPHIC::ViewPortHandle& viewportHandle, const QFE::GRAPHIC::ScissorRectHandle& scissorRectHandle,
	const QFE::GRAPHIC::DirectXResourceHandle& vertexBufferHandle, const std::vector<QFE::GRAPHIC::DirectXResourceHandle>& rootResources, 
	const std::vector<QFE::GRAPHIC::RenderTargetHandle>& renderTargets, const std::vector<D3D12_ROOT_PARAMETER_TYPE>& rootParameterTypes) {


	// 使用機能の取得
	QFE::GRAPHIC::DirectXCommandManager* commandManager = graphicEngine->GetDirectXCommandManager();
	QFE::GRAPHIC::RenderPass* renderPass = graphicEngine->GetRenderPass();
	QFE::GRAPHIC::DirectXResourceContainer* resourceContainer = graphicEngine->GetDirectXResourceContainer();
	QFE::GRAPHIC::GraphicPipelineManager* graphicPipelineManager = graphicEngine->GetGraphicPipelineManager();
	QFE::UniqueContainer<D3D12_VIEWPORT>& viewports = graphicEngine->GetViewports();
	QFE::UniqueContainer<D3D12_RECT>& scissorRects = graphicEngine->GetScissorRects();

	QFE::GRAPHIC::DirectXResourceHandle depthStencilBufferHandle = graphicEngine->GetDepthStencilBufferHandle();

	ID3D12GraphicsCommandList* commandList = commandManager->GetCommandList(D3D12_COMMAND_LIST_TYPE_DIRECT);

	// レンダーターゲットのバリアをレンダーターゲットに設定する前に、必要に応じてリソースの状態を遷移させる
	for(QFE::GRAPHIC::RenderTargetHandle renderTargetHandle : renderTargets) {
		QFE::GRAPHIC::DirectXResourceHandle renderTargetResourceHandle = renderPass->GetRenderTargetResourceHandle(renderTargetHandle);
		resourceContainer->TransitionResource(renderTargetResourceHandle, commandList, D3D12_RESOURCE_STATE_RENDER_TARGET);
	}

	renderPass->SetRenderTarget(
		commandList, depthStencilBufferHandle, renderTargets);

	commandList->RSSetViewports(1, viewports.GetData(static_cast<uint32_t>(viewportHandle)));
	commandList->RSSetScissorRects(1, scissorRects.GetData(static_cast<uint32_t>(scissorRectHandle)));

	commandList->SetPipelineState(graphicPipelineManager->GetPipelineState(psoHandle));
	commandList->SetGraphicsRootSignature(graphicPipelineManager->GetRootSignature(psoHandle));

	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView = resourceContainer->GetVertexBufferView(vertexBufferHandle);
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView);

	for (int i = 0; i < rootParameterTypes.size(); ++i) {
		D3D12_ROOT_PARAMETER_TYPE rootParameterType = rootParameterTypes[i];

		if (rootParameterType == D3D12_ROOT_PARAMETER_TYPE_CBV) {
			D3D12_GPU_VIRTUAL_ADDRESS gpuHandle = resourceContainer->GetGpuVirtualAddress(rootResources[i]);
			commandList->SetGraphicsRootConstantBufferView(static_cast<UINT>(i), gpuHandle);
		} else {
			D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = resourceContainer->GetDescriptorHandleGPU(rootResources[i], rootParameterType);
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

	size_t vertexCount = 0;
	if (!GetResourceArraySize(graphicEngine, vertexBufferHandle, vertexCount)) {
		assert(false);
		return false;
	}
	UINT vertexCountUINT = static_cast<UINT>(vertexCount);
	commandList->DrawInstanced(vertexCountUINT, 1, 0, 0);
	return true;
}

bool QFE::FRAMEWORK::DrawGraphicPSO(
	QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine, const QFE::GRAPHIC::PSOHandle& psoHandle,
	const QFE::GRAPHIC::ViewPortHandle& viewportHandle, const QFE::GRAPHIC::ScissorRectHandle& scissorRectHandle,
	const QFE::GRAPHIC::DirectXResourceHandle& vertexBufferHandle, const QFE::GRAPHIC::DirectXResourceHandle& indexBufferHandle,
	const std::vector<QFE::GRAPHIC::DirectXResourceHandle>& rootResources, const std::vector<QFE::GRAPHIC::RenderTargetHandle>& renderTargets, 
	const std::vector<D3D12_ROOT_PARAMETER_TYPE>& rootParameterTypes) {
	
	// 使用機能の取得
	QFE::GRAPHIC::DirectXCommandManager* commandManager = graphicEngine->GetDirectXCommandManager();
	QFE::GRAPHIC::RenderPass* renderPass = graphicEngine->GetRenderPass();
	QFE::GRAPHIC::DirectXResourceContainer* resourceContainer = graphicEngine->GetDirectXResourceContainer();
	QFE::GRAPHIC::GraphicPipelineManager* graphicPipelineManager = graphicEngine->GetGraphicPipelineManager();
	QFE::UniqueContainer<D3D12_VIEWPORT>& viewports = graphicEngine->GetViewports();
	QFE::UniqueContainer<D3D12_RECT>& scissorRects = graphicEngine->GetScissorRects();

	QFE::GRAPHIC::DirectXResourceHandle depthStencilBufferHandle = graphicEngine->GetDepthStencilBufferHandle();

	ID3D12GraphicsCommandList* commandList = commandManager->GetCommandList(D3D12_COMMAND_LIST_TYPE_DIRECT);

	// 必要ならレンダーターゲットの状態遷移
	for (QFE::GRAPHIC::RenderTargetHandle renderTargetHandle : renderTargets) {
		QFE::GRAPHIC::DirectXResourceHandle renderTargetResourceHandle = renderPass->GetRenderTargetResourceHandle(renderTargetHandle);
		resourceContainer->TransitionResource(renderTargetResourceHandle, commandList, D3D12_RESOURCE_STATE_RENDER_TARGET);
	}

	renderPass->SetRenderTarget(
		commandList, depthStencilBufferHandle, renderTargets);

	commandList->RSSetViewports(1, viewports.GetData(static_cast<uint32_t>(viewportHandle)));
	commandList->RSSetScissorRects(1, scissorRects.GetData(static_cast<uint32_t>(scissorRectHandle)));

	commandList->SetPipelineState(graphicPipelineManager->GetPipelineState(psoHandle));
	commandList->SetGraphicsRootSignature(graphicPipelineManager->GetRootSignature(psoHandle));

	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView = resourceContainer->GetVertexBufferView(vertexBufferHandle);
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView);

	// IndexBufferView を取得してセット
	D3D12_INDEX_BUFFER_VIEW indexBufferView = resourceContainer->GetIndexBufferView(indexBufferHandle);
	commandList->IASetIndexBuffer(&indexBufferView);

	for (int i = 0; i < rootParameterTypes.size(); ++i) {
		D3D12_ROOT_PARAMETER_TYPE rootParameterType = rootParameterTypes[i];

		if (rootParameterType == D3D12_ROOT_PARAMETER_TYPE_CBV) {
			D3D12_GPU_VIRTUAL_ADDRESS gpuHandle = resourceContainer->GetGpuVirtualAddress(rootResources[i]);
			commandList->SetGraphicsRootConstantBufferView(static_cast<UINT>(i), gpuHandle);
		} else {
			D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = resourceContainer->GetDescriptorHandleGPU(rootResources[i], rootParameterType);
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

	// インデックス数を取得して DrawIndexedInstanced を呼ぶ
	size_t indexCount = 0;
	if (!GetResourceArraySize(graphicEngine, indexBufferHandle, indexCount)) {
		assert(false);
		return false;
	}
	UINT indexCountUINT = static_cast<UINT>(indexCount);
	commandList->DrawIndexedInstanced(indexCountUINT, 1, 0, 0, 0);

	return true;
}

bool QFE::FRAMEWORK::DrawRayTracingPSO(
	QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine, const QFE::GRAPHIC::RTPSOHandle& rtpsoHandle,
	QFE::GRAPHIC::DirectXResourceHandle renderUavBuffer, const QFE::GRAPHIC::DirectXResourceHandle& cameraPositionBufferHandle,
	const std::vector<QFE::GRAPHIC::DirectXResourceHandle>& rootResources) {

	// 使用機能の取得
	QFE::GRAPHIC::RenderPass* renderPass = graphicEngine->GetRenderPass();
	QFE::GRAPHIC::DirectXCommandManager* commandManager = graphicEngine->GetDirectXCommandManager();
	QFE::GRAPHIC::RaytracingPipelineManager* raytracingPipelineManager = graphicEngine->GetRayTracingPipelineManager();
	QFE::GRAPHIC::DirectXResourceContainer* resourceContainer = graphicEngine->GetDirectXResourceContainer();
	QFE::GRAPHIC::RaytracingAccelerationStructure* accelerationStructure = graphicEngine->GetRaytracingAccelerationStructure();

	ID3D12GraphicsCommandList4* commandList4 = commandManager->GetCommandList4(D3D12_COMMAND_LIST_TYPE_DIRECT);
	ID3D12RootSignature* globalRootSignature = raytracingPipelineManager->GetRaytracingPipelineStateObject(rtpsoHandle)->GetRootSignature();
	ID3D12StateObject* rtpsoptr = raytracingPipelineManager->GetRaytracingPipelineStateObject(rtpsoHandle)->GetPipelineState();
	ID3D12Resource* rayGenShaderTable_ = raytracingPipelineManager->GetRaytracingPipelineStateObject(rtpsoHandle)->GetRayGenShaderTable();
	ID3D12Resource* missShaderTable_ = raytracingPipelineManager->GetRaytracingPipelineStateObject(rtpsoHandle)->GetMissShaderTable();
	ID3D12Resource* hitGroupShaderTable_ = raytracingPipelineManager->GetRaytracingPipelineStateObject(rtpsoHandle)->GetHitGroupShaderTable();

	ID3D12GraphicsCommandList* commandList = commandManager->GetCommandList(D3D12_COMMAND_LIST_TYPE_DIRECT);
	resourceContainer->TransitionResource(renderUavBuffer, commandList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	// 1. DXR用のパイプライン(RTPSO)とルートシグネチャをコマンドリストにセット
	commandList4->SetComputeRootSignature(globalRootSignature);
	commandList4->SetPipelineState1(rtpsoptr); // レイトレPSOはSetPipelineState1を使う

	// 2. ルートシグネチャへのリソースバインド
	D3D12_GPU_VIRTUAL_ADDRESS tlasResultBufferGPUHandle = accelerationStructure->GetTLASResultBuffer()->GetGPUVirtualAddress();
	commandList4->SetComputeRootShaderResourceView(1, tlasResultBufferGPUHandle);
	D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = resourceContainer->GetDescriptorHandleGPU(renderUavBuffer, QFE::GRAPHIC::ViewTypeFlags::UnorderedAccessView);

	D3D12_GPU_VIRTUAL_ADDRESS cameraGpuHandle = resourceContainer->GetGpuVirtualAddress(cameraPositionBufferHandle);
	commandList4->SetComputeRootConstantBufferView(0, cameraGpuHandle);

	// レンダーターゲットのバリアをレンダーターゲットに設定する前に、必要に応じてリソースの状態を遷移させる
	for (QFE::GRAPHIC::DirectXResourceHandle renderTargetHandle : rootResources) {
		resourceContainer->TransitionResource(renderTargetHandle, commandList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	}

	commandList4->SetComputeRootDescriptorTable(2, resourceContainer->GetDescriptorHandleGPU(rootResources[0], QFE::GRAPHIC::ViewTypeFlags::ShaderResourceView));
	commandList4->SetComputeRootDescriptorTable(3, resourceContainer->GetDescriptorHandleGPU(rootResources[1], QFE::GRAPHIC::ViewTypeFlags::ShaderResourceView));
	commandList4->SetComputeRootDescriptorTable(4, resourceContainer->GetDescriptorHandleGPU(rootResources[2], QFE::GRAPHIC::ViewTypeFlags::ShaderResourceView));
	commandList4->SetComputeRootDescriptorTable(5, resourceContainer->GetDescriptorHandleGPU(rootResources[3], QFE::GRAPHIC::ViewTypeFlags::ShaderResourceView));
	commandList4->SetComputeRootDescriptorTable(6, gpuHandle);

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

	// 5. コマンド発行
	commandList4->DispatchRays(&dispatchDesc);

	// 6. スワップチェーンのバックバッファにコピー
	resourceContainer->TransitionResource(renderUavBuffer, commandList, D3D12_RESOURCE_STATE_COPY_SOURCE);
	renderPass->TransitionCurrentBackBufferBarrier(
		commandList, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_DEST);
	commandList->CopyResource(
		renderPass->GetCurrentBackBuffer(), resourceContainer->GetResource(renderUavBuffer));
	renderPass->TransitionCurrentBackBufferBarrier(
		commandList, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_RENDER_TARGET);

	return true;
}

bool QFE::FRAMEWORK::DrawRayTracingPSO(QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine, const QFE::GRAPHIC::RTPSOHandle& rtpsoHandle, QFE::GRAPHIC::DirectXResourceHandle renderUavBuffer, const QFE::GRAPHIC::DirectXResourceHandle& cameraPositionBufferHandle, const std::vector<QFE::GRAPHIC::DirectXResourceHandle>& rootResources, const QFE::GRAPHIC::DirectXResourceHandle& renderTargetResourceHandle) {
	// 使用機能の取得
	QFE::GRAPHIC::RenderPass* renderPass = graphicEngine->GetRenderPass();
	QFE::GRAPHIC::DirectXCommandManager* commandManager = graphicEngine->GetDirectXCommandManager();
	QFE::GRAPHIC::RaytracingPipelineManager* raytracingPipelineManager = graphicEngine->GetRayTracingPipelineManager();
	QFE::GRAPHIC::DirectXResourceContainer* resourceContainer = graphicEngine->GetDirectXResourceContainer();
	QFE::GRAPHIC::RaytracingAccelerationStructure* accelerationStructure = graphicEngine->GetRaytracingAccelerationStructure();

	ID3D12GraphicsCommandList4* commandList4 = commandManager->GetCommandList4(D3D12_COMMAND_LIST_TYPE_DIRECT);
	ID3D12RootSignature* globalRootSignature = raytracingPipelineManager->GetRaytracingPipelineStateObject(rtpsoHandle)->GetRootSignature();
	ID3D12StateObject* rtpsoptr = raytracingPipelineManager->GetRaytracingPipelineStateObject(rtpsoHandle)->GetPipelineState();
	ID3D12Resource* rayGenShaderTable_ = raytracingPipelineManager->GetRaytracingPipelineStateObject(rtpsoHandle)->GetRayGenShaderTable();
	ID3D12Resource* missShaderTable_ = raytracingPipelineManager->GetRaytracingPipelineStateObject(rtpsoHandle)->GetMissShaderTable();
	ID3D12Resource* hitGroupShaderTable_ = raytracingPipelineManager->GetRaytracingPipelineStateObject(rtpsoHandle)->GetHitGroupShaderTable();

	ID3D12GraphicsCommandList* commandList = commandManager->GetCommandList(D3D12_COMMAND_LIST_TYPE_DIRECT);
	resourceContainer->TransitionResource(renderUavBuffer, commandList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	// 1. DXR用のパイプライン(RTPSO)とルートシグネチャをコマンドリストにセット
	commandList4->SetComputeRootSignature(globalRootSignature);
	commandList4->SetPipelineState1(rtpsoptr); // レイトレPSOはSetPipelineState1を使う

	// 2. ルートシグネチャへのリソースバインド
	D3D12_GPU_VIRTUAL_ADDRESS tlasResultBufferGPUHandle = accelerationStructure->GetTLASResultBuffer()->GetGPUVirtualAddress();
	commandList4->SetComputeRootShaderResourceView(1, tlasResultBufferGPUHandle);
	D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = resourceContainer->GetDescriptorHandleGPU(renderUavBuffer, QFE::GRAPHIC::ViewTypeFlags::UnorderedAccessView);

	D3D12_GPU_VIRTUAL_ADDRESS cameraGpuHandle = resourceContainer->GetGpuVirtualAddress(cameraPositionBufferHandle);
	commandList4->SetComputeRootConstantBufferView(0, cameraGpuHandle);

	// レンダーターゲットのバリアをレンダーターゲットに設定する前に、必要に応じてリソースの状態を遷移させる
	for (QFE::GRAPHIC::DirectXResourceHandle renderTargetHandle : rootResources) {
		resourceContainer->TransitionResource(renderTargetHandle, commandList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	}

	commandList4->SetComputeRootDescriptorTable(2, resourceContainer->GetDescriptorHandleGPU(rootResources[0], QFE::GRAPHIC::ViewTypeFlags::ShaderResourceView));
	commandList4->SetComputeRootDescriptorTable(3, resourceContainer->GetDescriptorHandleGPU(rootResources[1], QFE::GRAPHIC::ViewTypeFlags::ShaderResourceView));
	commandList4->SetComputeRootDescriptorTable(4, resourceContainer->GetDescriptorHandleGPU(rootResources[2], QFE::GRAPHIC::ViewTypeFlags::ShaderResourceView));
	commandList4->SetComputeRootDescriptorTable(5, resourceContainer->GetDescriptorHandleGPU(rootResources[3], QFE::GRAPHIC::ViewTypeFlags::ShaderResourceView));
	commandList4->SetComputeRootDescriptorTable(6, gpuHandle);

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

	// 5. コマンド発行
	commandList4->DispatchRays(&dispatchDesc);

	// 6. スワップチェーンのバックバッファにコピー
	resourceContainer->TransitionResource(renderUavBuffer, commandList, D3D12_RESOURCE_STATE_COPY_SOURCE);
	resourceContainer->TransitionResource(renderTargetResourceHandle, commandList, D3D12_RESOURCE_STATE_COPY_DEST);
	commandList->CopyResource(
		resourceContainer->GetResource(renderTargetResourceHandle), resourceContainer->GetResource(renderUavBuffer));
	resourceContainer->TransitionResource(renderTargetResourceHandle, commandList,D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	return true;
}

bool QFE::FRAMEWORK::TestRayTracingPSO(
	QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine,
	const QFE::GRAPHIC::RTPSOHandle& rtpsoHandle,
	QFE::GRAPHIC::DirectXResourceHandle renderUavBuffer, 
	const QFE::GRAPHIC::DirectXResourceHandle& cameraPositionBufferHandle, 
	const QFE::GRAPHIC::DirectXResourceHandle& indexBufferHandle, 
	const QFE::GRAPHIC::DirectXResourceHandle& uvBufferHandle, 
	const QFE::GRAPHIC::DirectXResourceHandle& instanceMetaBufferHandle,
	const QFE::GRAPHIC::DirectXResourceHandle& firstTextureBufferHandle,
	const std::vector<QFE::GRAPHIC::DirectXResourceHandle>& rootResources) {
	// 使用機能の取得
	QFE::GRAPHIC::RenderPass* renderPass = graphicEngine->GetRenderPass();
	QFE::GRAPHIC::DirectXCommandManager* commandManager = graphicEngine->GetDirectXCommandManager();
	QFE::GRAPHIC::RaytracingPipelineManager* raytracingPipelineManager = graphicEngine->GetRayTracingPipelineManager();
	QFE::GRAPHIC::DirectXResourceContainer* resourceContainer = graphicEngine->GetDirectXResourceContainer();
	QFE::GRAPHIC::RaytracingAccelerationStructure* accelerationStructure = graphicEngine->GetRaytracingAccelerationStructure();

	ID3D12GraphicsCommandList4* commandList4 = commandManager->GetCommandList4(D3D12_COMMAND_LIST_TYPE_DIRECT);
	ID3D12RootSignature* globalRootSignature = raytracingPipelineManager->GetRaytracingPipelineStateObject(rtpsoHandle)->GetRootSignature();
	ID3D12StateObject* rtpsoptr = raytracingPipelineManager->GetRaytracingPipelineStateObject(rtpsoHandle)->GetPipelineState();
	ID3D12Resource* rayGenShaderTable_ = raytracingPipelineManager->GetRaytracingPipelineStateObject(rtpsoHandle)->GetRayGenShaderTable();
	ID3D12Resource* missShaderTable_ = raytracingPipelineManager->GetRaytracingPipelineStateObject(rtpsoHandle)->GetMissShaderTable();
	ID3D12Resource* hitGroupShaderTable_ = raytracingPipelineManager->GetRaytracingPipelineStateObject(rtpsoHandle)->GetHitGroupShaderTable();

	ID3D12GraphicsCommandList* commandList = commandManager->GetCommandList(D3D12_COMMAND_LIST_TYPE_DIRECT);
	resourceContainer->TransitionResource(renderUavBuffer, commandList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	// 1. DXR用のパイプライン(RTPSO)とルートシグネチャをコマンドリストにセット
	commandList4->SetComputeRootSignature(globalRootSignature);
	commandList4->SetPipelineState1(rtpsoptr); // レイトレPSOはSetPipelineState1を使う

	// 2. ルートシグネチャへのリソースバインド
	D3D12_GPU_VIRTUAL_ADDRESS tlasResultBufferGPUHandle = accelerationStructure->GetTLASResultBuffer()->GetGPUVirtualAddress();
	commandList4->SetComputeRootShaderResourceView(5, tlasResultBufferGPUHandle);
	D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = resourceContainer->GetDescriptorHandleGPU(renderUavBuffer, QFE::GRAPHIC::ViewTypeFlags::UnorderedAccessView);

	commandList4->SetComputeRootDescriptorTable(0, resourceContainer->GetDescriptorHandleGPU(indexBufferHandle, QFE::GRAPHIC::ViewTypeFlags::ShaderResourceView));
	commandList4->SetComputeRootDescriptorTable(1, resourceContainer->GetDescriptorHandleGPU(uvBufferHandle, QFE::GRAPHIC::ViewTypeFlags::ShaderResourceView));
	commandList4->SetComputeRootDescriptorTable(2, resourceContainer->GetDescriptorHandleGPU(instanceMetaBufferHandle, QFE::GRAPHIC::ViewTypeFlags::ShaderResourceView));
	commandList4->SetComputeRootDescriptorTable(3, resourceContainer->GetDescriptorHandleGPU(firstTextureBufferHandle, QFE::GRAPHIC::ViewTypeFlags::ShaderResourceView));

	D3D12_GPU_VIRTUAL_ADDRESS cameraGpuHandle = resourceContainer->GetGpuVirtualAddress(cameraPositionBufferHandle);
	commandList4->SetComputeRootConstantBufferView(4, cameraGpuHandle);

	// レンダーターゲットのバリアをレンダーターゲットに設定する前に、必要に応じてリソースの状態を遷移させる
	for (QFE::GRAPHIC::DirectXResourceHandle renderTargetHandle : rootResources) {
		resourceContainer->TransitionResource(renderTargetHandle, commandList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	}

	commandList4->SetComputeRootDescriptorTable(6, resourceContainer->GetDescriptorHandleGPU(rootResources[0], QFE::GRAPHIC::ViewTypeFlags::ShaderResourceView));
	commandList4->SetComputeRootDescriptorTable(7, resourceContainer->GetDescriptorHandleGPU(rootResources[1], QFE::GRAPHIC::ViewTypeFlags::ShaderResourceView));
	commandList4->SetComputeRootDescriptorTable(8, resourceContainer->GetDescriptorHandleGPU(rootResources[2], QFE::GRAPHIC::ViewTypeFlags::ShaderResourceView));
	commandList4->SetComputeRootDescriptorTable(9, resourceContainer->GetDescriptorHandleGPU(rootResources[3], QFE::GRAPHIC::ViewTypeFlags::ShaderResourceView));
	commandList4->SetComputeRootDescriptorTable(10, gpuHandle);
	

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

	// 5. コマンド発行
	commandList4->DispatchRays(&dispatchDesc);

	// 6. スワップチェーンのバックバッファにコピー
	resourceContainer->TransitionResource(renderUavBuffer, commandList, D3D12_RESOURCE_STATE_COPY_SOURCE);
	renderPass->TransitionCurrentBackBufferBarrier(
		commandList, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_DEST);
	commandList->CopyResource(
		renderPass->GetCurrentBackBuffer(), resourceContainer->GetResource(renderUavBuffer));
	renderPass->TransitionCurrentBackBufferBarrier(
		commandList, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_RENDER_TARGET);

	return true;
}

bool QFE::FRAMEWORK::LoadTextureFromFile(QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine, const std::string& filePath, QFE::GRAPHIC::DirectXResourceHandle& outTextureHandle) {
	QFE::GRAPHIC::TextureLoader* textureLoader = graphicEngine->GetTextureLoader();
	
	outTextureHandle = textureLoader->LoadTexture(filePath);
	if(outTextureHandle == QFE::GRAPHIC::DirectXResourceHandle::Invalid) {
		return false;
	}
	return true;
}
