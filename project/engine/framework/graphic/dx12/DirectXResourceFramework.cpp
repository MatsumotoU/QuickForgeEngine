#include "DirectXResourceFramework.h"

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
#include "../../../resources/Shaders/ShaderStructs/hlslTypeToCpp.h"

bool QFE::FRAMEWORK::GetWhite1x1TextureHandle(
	QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine, QFE::GRAPHIC::DirectXResourceHandle& outTextureHandle) {

	QFE::GRAPHIC::TextureLoader* textureLoader = graphicEngine->GetTextureLoader();
	outTextureHandle = textureLoader->GetDummyWhite1x1TextureHandle();
	// 成否の確認
	if (outTextureHandle == QFE::GRAPHIC::DirectXResourceHandle::Invalid) {
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

bool QFE::FRAMEWORK::UAVBarrierTransition(QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine, const QFE::GRAPHIC::DirectXResourceHandle& resourceHandle) {
	QFE::GRAPHIC::DirectXResourceContainer* resourceContainer = graphicEngine->GetDirectXResourceContainer();
	QFE::GRAPHIC::DirectXCommandManager* commandManager = graphicEngine->GetDirectXCommandManager();

	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.UAV.pResource = resourceContainer->GetResource(resourceHandle);
	commandManager->GetCommandList(D3D12_COMMAND_LIST_TYPE_DIRECT)->ResourceBarrier(1, &barrier);
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
	QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine, QFE::GRAPHIC::RenderTargetHandle renderTargetHandle, QFE::GRAPHIC::DirectXResourceHandle& outResourceHandle) {

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
	if (outUAVBufferHandle == QFE::GRAPHIC::DirectXResourceHandle::Invalid) {
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
	transformMatrix.World = QFE::MATH::Matrix4x4::MakeIdentity4x4();
	transformMatrix.WVP = QFE::MATH::Matrix4x4::MakeIdentity4x4();
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
	if (outCameraPosBufferHandle == QFE::GRAPHIC::DirectXResourceHandle::Invalid) {
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
	if (outVertexBufferHandle == QFE::GRAPHIC::DirectXResourceHandle::Invalid) {
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
	if (outBLASHandle == QFE::GRAPHIC::BLASHandle::Invalid) {
		QFE_LOG("Failed to create BLAS for model: " + name);
		return false;
	}
	return true;
}

bool QFE::FRAMEWORK::LoadTextureFromFile(QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine, const std::string& filePath, QFE::GRAPHIC::DirectXResourceHandle& outTextureHandle) {
	QFE::GRAPHIC::TextureLoader* textureLoader = graphicEngine->GetTextureLoader();

	outTextureHandle = textureLoader->LoadTexture(filePath);
	if (outTextureHandle == QFE::GRAPHIC::DirectXResourceHandle::Invalid) {
		return false;
	}
	return true;
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

bool QFE::FRAMEWORK::EnsureBufferCapacityAndUpload(
	QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine, QFE::GRAPHIC::DirectXResourceHandle& inOutHandle,
	const void* data, size_t byteSize, UINT elementStride, const std::string& name) {

	if (!graphicEngine) {
		return false;
	}
	auto* device = graphicEngine->GetDirectXDevice();
	auto* rc = graphicEngine->GetDirectXResourceContainer();

	// サイズが0の場合は、無効化する
	if (byteSize == 0) {
		QFE::GRAPHIC::DirectXResourceHandle invalidHandle = QFE::GRAPHIC::DirectXResourceHandle::Invalid;
		return true;
	}

	// 新規作成 or 既存サイズ取得
	bool needCreateView = false;
	if (inOutHandle == QFE::GRAPHIC::DirectXResourceHandle::Invalid) {
		needCreateView = true;
	} else {
		size_t curSize = rc->GetResourceSizeInBytes(inOutHandle);
		if (curSize < byteSize) {
			// 既存バッファでは小さい -> 新規作成して差し替え
			needCreateView = true;
		}
	}

	if (needCreateView) {
		// 新しいバッファを作成してハンドルを差し替える
		QFE::GRAPHIC::DirectXResourceHandle newHandle = rc->CreateBuffer(device->GetDevice(), byteSize);
		if (newHandle == QFE::GRAPHIC::DirectXResourceHandle::Invalid) return false;

		rc->MapResource(newHandle);
		if (byteSize > 0 && data) {
			void* mapped = rc->GetMappedData<void>(newHandle);
			if (mapped) memcpy(mapped, data, byteSize);
		}
		rc->SetResourceName(newHandle, QFE::ConvertString(name));
		rc->SetResourceStrideInBytes(newHandle, elementStride);

		// SRV を作る（Shader4ComponentMapping を忘れずに）
		QFE::GRAPHIC::CreateViewInfo viewInfo{};
		viewInfo.viewType = QFE::GRAPHIC::ViewTypeFlags::ShaderResourceView;
		viewInfo.srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		viewInfo.srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		viewInfo.srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		viewInfo.srvDesc.Buffer.NumElements = static_cast<UINT>(byteSize / elementStride);
		viewInfo.srvDesc.Buffer.StructureByteStride = elementStride;
		viewInfo.srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
		rc->CreateResourceView(newHandle, viewInfo);

		// 既存ハンドルがあれば置き換える（リソース解放は container に任せる想定）
		inOutHandle = newHandle;
		return true;
	} else {
		// 既存バッファを再利用してデータ更新だけ行う（Map + memcpy）
		rc->MapResource(inOutHandle);
		if (byteSize > 0 && data) {
			void* mapped = rc->GetMappedData<void>(inOutHandle);
			if (mapped) memcpy(mapped, data, byteSize);
		}
		// strideは念のため再設定
		rc->SetResourceStrideInBytes(inOutHandle, elementStride);
		return true;
	}
	
}

bool QFE::FRAMEWORK::UploadGlobalMeshBuffers(
	QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine, const std::vector<float>& globalUVs,
	const std::vector<uint32_t>& globalTriIndices, const std::vector<InstanceMetaCPU>& instanceMeta,
	QFE::GRAPHIC::DirectXResourceHandle& outUVHandle, QFE::GRAPHIC::DirectXResourceHandle& outTriHandle, 
	QFE::GRAPHIC::DirectXResourceHandle& outInstanceMetaHandle) {

	if (!graphicEngine) return false;
	auto* device = graphicEngine->GetDirectXDevice();
	auto* rc = graphicEngine->GetDirectXResourceContainer();

	// 1) UV バッファ
	{
		size_t byteSize = globalUVs.size() * sizeof(float);
		auto handle = rc->CreateBuffer(device->GetDevice(), byteSize);
		if (handle == QFE::GRAPHIC::DirectXResourceHandle::Invalid) return false;

		rc->MapResource(handle);
		float* mapped = rc->GetMappedData<float>(handle);
		if (mapped && !globalUVs.empty()) {
			memcpy(mapped, globalUVs.data(), byteSize);
		}
		rc->SetResourceName(handle, QFE::ConvertString(std::string("GlobalUVs")));
		// 要素ストライドは float2
		rc->SetResourceStrideInBytes(handle, static_cast<UINT>(sizeof(float) * 2));

		// SRV 作成（Shader4ComponentMapping を明示的に設定）
		QFE::GRAPHIC::CreateViewInfo viewInfo{};
		viewInfo.viewType = QFE::GRAPHIC::ViewTypeFlags::ShaderResourceView;
		viewInfo.srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		viewInfo.srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		viewInfo.srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		viewInfo.srvDesc.Buffer.NumElements = static_cast<UINT>(globalUVs.size() / 2);
		viewInfo.srvDesc.Buffer.StructureByteStride = sizeof(float) * 2;
		viewInfo.srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
		rc->CreateResourceView(handle, viewInfo);

		outUVHandle = handle;
	}

	// 2) TriIndices バッファ（flattened uint triplets）
	{
		size_t byteSize = globalTriIndices.size() * sizeof(uint32_t);
		auto handle = rc->CreateBuffer(device->GetDevice(), byteSize);
		if (handle == QFE::GRAPHIC::DirectXResourceHandle::Invalid) return false;

		rc->MapResource(handle);
		uint32_t* mapped = rc->GetMappedData<uint32_t>(handle);
		if (mapped && !globalTriIndices.empty()) {
			memcpy(mapped, globalTriIndices.data(), byteSize);
		}
		rc->SetResourceName(handle, QFE::ConvertString(std::string("GlobalTriIndices")));
		// tri ごとの stride は 3 * uint32
		rc->SetResourceStrideInBytes(handle, static_cast<UINT>(sizeof(uint32_t) * 3));

		QFE::GRAPHIC::CreateViewInfo viewInfo{};
		viewInfo.viewType = QFE::GRAPHIC::ViewTypeFlags::ShaderResourceView;
		viewInfo.srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		viewInfo.srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		viewInfo.srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		viewInfo.srvDesc.Buffer.NumElements = static_cast<UINT>(globalTriIndices.size() / 3);
		viewInfo.srvDesc.Buffer.StructureByteStride = sizeof(uint32_t) * 3;
		viewInfo.srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
		rc->CreateResourceView(handle, viewInfo);

		outTriHandle = handle;
	}

	// 3) InstanceMeta バッファ
	{
		size_t byteSize = instanceMeta.size() * sizeof(InstanceMetaCPU);
		auto handle = rc->CreateBuffer(device->GetDevice(), byteSize);
		if (handle == QFE::GRAPHIC::DirectXResourceHandle::Invalid) return false;

		rc->MapResource(handle);
		InstanceMetaCPU* mapped = rc->GetMappedData<InstanceMetaCPU>(handle);
		if (mapped && !instanceMeta.empty()) {
			memcpy(mapped, instanceMeta.data(), byteSize);
		}
		rc->SetResourceName(handle, QFE::ConvertString(std::string("InstanceMeta")));
		rc->SetResourceStrideInBytes(handle, static_cast<UINT>(sizeof(InstanceMetaCPU)));

		QFE::GRAPHIC::CreateViewInfo viewInfo{};
		viewInfo.viewType = QFE::GRAPHIC::ViewTypeFlags::ShaderResourceView;
		viewInfo.srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		viewInfo.srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		viewInfo.srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		viewInfo.srvDesc.Buffer.NumElements = static_cast<UINT>(instanceMeta.size());
		viewInfo.srvDesc.Buffer.StructureByteStride = sizeof(InstanceMetaCPU);
		viewInfo.srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
		rc->CreateResourceView(handle, viewInfo);

		outInstanceMetaHandle = handle;
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