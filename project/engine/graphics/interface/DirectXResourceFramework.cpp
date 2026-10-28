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
#include "core/string/MyString.h"
#include "resources/shaders/shaderStructs/hlslTypeToCpp.h"

namespace {
	[[nodiscard]] QFE::GRAPHIC::D3D12GraphicEngine* ResolveD3D12GraphicEngine(
		QFE::GRAPHIC::IGraphicEngine* graphicEngine) {
		auto* d3d12GraphicEngine =
			dynamic_cast<QFE::GRAPHIC::D3D12GraphicEngine*>(graphicEngine);
		if (d3d12GraphicEngine == nullptr || !d3d12GraphicEngine->IsCoreSystemAvailable()) {
			QFE_LOG("A DirectX 12 graphic engine with an initialized core system is required.");
			return nullptr;
		}
		return d3d12GraphicEngine;
	}
}

bool QFE::GRAPHIC::GetWhite1x1TextureHandle(
	QFE::GRAPHIC::IGraphicEngine* graphicEngine, QFE::GRAPHIC::DirectXResourceHandle& outTextureHandle) {

	QFE::GRAPHIC::TextureLoader* textureLoader = ResolveD3D12GraphicEngine(graphicEngine)->GetTextureLoader();
	outTextureHandle = textureLoader->GetDummyWhite1x1TextureHandle();
	// 成否の確認
	if (outTextureHandle == QFE::GRAPHIC::DirectXResourceHandle::Invalid) {
		QFE_LOG("Failed to get white 1x1 texture handle.");
		return false;
	}
	return true;
}

bool QFE::GRAPHIC::GetBlackCubeMapTextureHandle(
	QFE::GRAPHIC::IGraphicEngine* graphicEngine, QFE::GRAPHIC::DirectXResourceHandle& outTextureHandle) {

	QFE::GRAPHIC::TextureLoader* textureLoader = ResolveD3D12GraphicEngine(graphicEngine)->GetTextureLoader();
	outTextureHandle = textureLoader->GetDummyBlackCubeMapHandle();
	// 成否の確認
	if (outTextureHandle == QFE::GRAPHIC::DirectXResourceHandle::Invalid) {
		QFE_LOG("Failed to get black cube map texture handle.");
		return false;
	}
	return true;
}

bool QFE::GRAPHIC::UAVBarrierTransition(QFE::GRAPHIC::IGraphicEngine* graphicEngine, const QFE::GRAPHIC::DirectXResourceHandle& resourceHandle) {
	QFE::GRAPHIC::DirectXResourceContainer* resourceContainer = ResolveD3D12GraphicEngine(graphicEngine)->GetDirectXResourceContainer();
	QFE::GRAPHIC::DirectXCommandManager* commandManager = ResolveD3D12GraphicEngine(graphicEngine)->GetDirectXCommandManager();

	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.UAV.pResource = resourceContainer->GetResource(resourceHandle);
	commandManager->GetCommandList(D3D12_COMMAND_LIST_TYPE_DIRECT)->ResourceBarrier(1, &barrier);
	return true;
}

bool QFE::GRAPHIC::GetResourceArraySize(
	QFE::GRAPHIC::IGraphicEngine* graphicEngine, QFE::GRAPHIC::DirectXResourceHandle resourceHandle, size_t& outResourceArraySize) {

	QFE::GRAPHIC::DirectXResourceContainer* resourceContainer = ResolveD3D12GraphicEngine(graphicEngine)->GetDirectXResourceContainer();

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

bool QFE::GRAPHIC::GetRenderResourceHandle(
	QFE::GRAPHIC::IGraphicEngine* graphicEngine, QFE::GRAPHIC::RenderTargetHandle renderTargetHandle, QFE::GRAPHIC::DirectXResourceHandle& outResourceHandle) {

	QFE::GRAPHIC::RenderPass* renderPass = ResolveD3D12GraphicEngine(graphicEngine)->GetRenderPass();
	outResourceHandle = renderPass->GetRenderTargetResourceHandle(renderTargetHandle);
	// 成否の確認
	if (outResourceHandle == QFE::GRAPHIC::DirectXResourceHandle::Invalid) {
		QFE_LOG("Failed to get render resource handle.");
		return false;
	}
	return true;
}

bool QFE::GRAPHIC::GetDepthStencilResourceHandle(
	QFE::GRAPHIC::IGraphicEngine* graphicEngine, QFE::GRAPHIC::DirectXResourceHandle& outDepthStencilHandle) {

	QFE::GRAPHIC::DirectXResourceHandle depthStencilHandle = ResolveD3D12GraphicEngine(graphicEngine)->GetDepthStencilBufferHandle();
	outDepthStencilHandle = depthStencilHandle;
	// 成否の確認
	if (outDepthStencilHandle == QFE::GRAPHIC::DirectXResourceHandle::Invalid) {
		QFE_LOG("Failed to get depth stencil resource handle.");
		return false;
	}
	return true;
}

bool QFE::GRAPHIC::CreateUAVBuffer(
	QFE::GRAPHIC::IGraphicEngine* graphicEngine, QFE::GRAPHIC::DirectXResourceHandle& outUAVBufferHandle,
	uint32_t width, uint32_t height, const std::wstring& name) {

	QFE::GRAPHIC::DirectXDevice* directXDevice = ResolveD3D12GraphicEngine(graphicEngine)->GetDirectXDevice();
	QFE::GRAPHIC::DirectXResourceContainer* resourceContainer = ResolveD3D12GraphicEngine(graphicEngine)->GetDirectXResourceContainer();

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

bool QFE::GRAPHIC::CreateObject3dGBufferRootResources(
	QFE::GRAPHIC::IGraphicEngine* graphicEngine, QFE::GRAPHIC::DirectXResourceHandle* rootResources, size_t rootResourcesSize) {

	// ルートリソースの数を確認
	if (rootResourcesSize < 3) {
		QFE_LOG("rootResourcesSize is too small. It should be at least 3.");
		return false;
	}

	// リソースアロケータを取得
	QFE::GRAPHIC::DirectXResourceAllocator* resourceAllocator = ResolveD3D12GraphicEngine(graphicEngine)->GetDirectXResourceAllocator();
	// テクスチャマネージャを取得
	QFE::GRAPHIC::TextureLoader* textureLoader = ResolveD3D12GraphicEngine(graphicEngine)->GetTextureLoader();

	// EulerTransform
	ObjectTransformGpuData transformMatrix;
	transformMatrix.World = QFE::MATH::Matrix4x4::MakeIdentity4x4();
	transformMatrix.WVP = QFE::MATH::Matrix4x4::MakeIdentity4x4();
	QFE::GRAPHIC::DirectXResourceHandle transformMatrixBufferHandle =
		resourceAllocator->AllocateConstantBuffer<ObjectTransformGpuData>("TransformMatrixBuffer");
	// Material
	MaterialGpuData material;
	material.color = { 1.0f, 1.0f, 1.0f, 1.0f };
	QFE::GRAPHIC::DirectXResourceHandle materialBufferHandle =
		resourceAllocator->AllocateConstantBuffer<MaterialGpuData>("MaterialBuffer");

	//Texture
	QFE::GRAPHIC::DirectXResourceHandle textureHandle = textureLoader->GetDummyWhite1x1TextureHandle();

	// ルートリソースに設定
	rootResources[0] = transformMatrixBufferHandle;
	rootResources[1] = materialBufferHandle;
	rootResources[2] = textureHandle;

	return true;
}

bool QFE::GRAPHIC::TransitionResourceToState(
	QFE::GRAPHIC::IGraphicEngine* graphicEngine,
	const QFE::GRAPHIC::DirectXResourceHandle& resourceHandle,
	D3D12_RESOURCE_STATES newState) {

	QFE::GRAPHIC::DirectXCommandManager* commandManager = ResolveD3D12GraphicEngine(graphicEngine)->GetDirectXCommandManager();
	QFE::GRAPHIC::DirectXResourceContainer* resourceContainer = ResolveD3D12GraphicEngine(graphicEngine)->GetDirectXResourceContainer();

	// コマンドリストを取得
	ID3D12GraphicsCommandList* commandList = commandManager->GetCommandList(D3D12_COMMAND_LIST_TYPE_DIRECT);
	return resourceContainer->TransitionResource(resourceHandle, commandList, newState);
}

bool QFE::GRAPHIC::CreateCameraPosBuffer(
	QFE::GRAPHIC::IGraphicEngine* graphicEngine, const std::wstring& name, QFE::GRAPHIC::DirectXResourceHandle& outCameraPosBufferHandle) {

	// リソースアロケータを取得
	QFE::GRAPHIC::DirectXResourceAllocator* resourceAllocator = ResolveD3D12GraphicEngine(graphicEngine)->GetDirectXResourceAllocator();
	// カメラ位置バッファを作成
	outCameraPosBufferHandle = resourceAllocator->AllocateConstantBuffer<QFE::MATH::Vector3>(ConvertString(name));
	// 成否の確認
	if (outCameraPosBufferHandle == QFE::GRAPHIC::DirectXResourceHandle::Invalid) {
		QFE_LOG("Failed to create camera position buffer.");
		return false;
	}
	return true;
}

bool QFE::GRAPHIC::AllocateConstantBuffer(
	QFE::GRAPHIC::IGraphicEngine* graphicEngine,
	const std::wstring& name, size_t bufferSize,
	QFE::GRAPHIC::DirectXResourceHandle& outConstantBufferHandle) {

	// リソースアロケータを取得
	QFE::GRAPHIC::DirectXResourceAllocator* resourceAllocator = ResolveD3D12GraphicEngine(graphicEngine)->GetDirectXResourceAllocator();
	// 定数バッファを作成
	outConstantBufferHandle = resourceAllocator->AllocateRawConstantBuffer(bufferSize, ConvertString(name));
	// 成否の確認
	if (outConstantBufferHandle == QFE::GRAPHIC::DirectXResourceHandle::Invalid) {
		QFE_LOG("Failed to allocate constant buffer: " + QFE::ConvertString(name));
		return false;
	}
	return true;
}

bool QFE::GRAPHIC::AllocateStructuredBuffer(
	QFE::GRAPHIC::IGraphicEngine* graphicEngine,
	const std::wstring& name, size_t elementCount, size_t elementStride,
	QFE::GRAPHIC::DirectXResourceHandle& outStructuredBufferHandle) {

	// リソースアロケータを取得
	QFE::GRAPHIC::DirectXResourceAllocator* resourceAllocator = ResolveD3D12GraphicEngine(graphicEngine)->GetDirectXResourceAllocator();
	// 構造化バッファを作成
	outStructuredBufferHandle = resourceAllocator->AllocateRawStructuredBuffer(elementCount * elementStride, elementStride, ConvertString(name));
	// 成否の確認
	if (outStructuredBufferHandle == QFE::GRAPHIC::DirectXResourceHandle::Invalid) {
		QFE_LOG("Failed to allocate structured buffer: " + QFE::ConvertString(name));
		return false;
	}
	return true;
}

bool QFE::GRAPHIC::CreateReadWriteStructuredBuffer(
	QFE::GRAPHIC::IGraphicEngine* graphicEngine,
	const std::wstring& name,
	size_t elementCount,
	size_t elementStride,
	QFE::GRAPHIC::DirectXResourceHandle& outStructuredBufferHandle) {
	outStructuredBufferHandle = QFE::GRAPHIC::DirectXResourceHandle::Invalid;
	auto* d3d12GraphicEngine = ResolveD3D12GraphicEngine(graphicEngine);
	if (d3d12GraphicEngine == nullptr || elementCount == 0 || elementStride == 0) {
		return false;
	}

	QFE::GRAPHIC::DirectXResourceContainer* resourceContainer =
		d3d12GraphicEngine->GetDirectXResourceContainer();
	QFE::GRAPHIC::DirectXDevice* directXDevice = d3d12GraphicEngine->GetDirectXDevice();
	if (resourceContainer == nullptr || directXDevice == nullptr) {
		return false;
	}

	D3D12_RESOURCE_DESC bufferDesc{};
	bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	bufferDesc.Width = static_cast<UINT64>(elementCount * elementStride);
	bufferDesc.Height = 1;
	bufferDesc.DepthOrArraySize = 1;
	bufferDesc.MipLevels = 1;
	bufferDesc.SampleDesc.Count = 1;
	bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	bufferDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

	const QFE::GRAPHIC::DirectXResourceHandle handle = resourceContainer->CreateResource(
		directXDevice->GetDevice(), bufferDesc,
		D3D12_RESOURCE_STATE_COMMON, D3D12_HEAP_TYPE_DEFAULT);
	if (handle == QFE::GRAPHIC::DirectXResourceHandle::Invalid) {
		QFE_LOG("Failed to create read-write structured buffer: " + QFE::ConvertString(name));
		return false;
	}

	QFE::GRAPHIC::CreateViewInfo viewInfo{};
	viewInfo.viewType = QFE::GRAPHIC::ViewTypeFlags::ShaderResourceView |
		QFE::GRAPHIC::ViewTypeFlags::UnorderedAccessView;
	viewInfo.srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	viewInfo.srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	viewInfo.srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	viewInfo.srvDesc.Buffer.NumElements = static_cast<UINT>(elementCount);
	viewInfo.srvDesc.Buffer.StructureByteStride = static_cast<UINT>(elementStride);
	viewInfo.uavDesc.Format = DXGI_FORMAT_UNKNOWN;
	viewInfo.uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	viewInfo.uavDesc.Buffer.NumElements = static_cast<UINT>(elementCount);
	viewInfo.uavDesc.Buffer.StructureByteStride = static_cast<UINT>(elementStride);
	resourceContainer->CreateResourceView(handle, viewInfo);
	resourceContainer->SetResourceStrideInBytes(handle, elementStride);
	resourceContainer->SetResourceName(handle, name);
	outStructuredBufferHandle = handle;
	return true;
}

void QFE::GRAPHIC::UpdateObject3dWVPMatrix(
	QFE::GRAPHIC::IGraphicEngine* graphicEngine, const QFE::GRAPHIC::DirectXResourceHandle& transformMatrixBufferHandle,
	const QFE::MATH::EulerTransform& transform, const QFE::MATH::Matrix4x4& viewProjectionMatrix) {
	UpdateObject3dWVPMatrix(
		graphicEngine,
		transformMatrixBufferHandle,
		QFE::MATH::Matrix4x4::MakeAffineMatrix(transform),
		viewProjectionMatrix);
}

void QFE::GRAPHIC::UpdateObject3dWVPMatrix(
	QFE::GRAPHIC::IGraphicEngine* graphicEngine, const QFE::GRAPHIC::DirectXResourceHandle& transformMatrixBufferHandle,
	const QFE::MATH::Matrix4x4& worldMatrix, const QFE::MATH::Matrix4x4& viewProjectionMatrix) {
	ObjectTransformGpuData* transformMatrixData =
		ResolveD3D12GraphicEngine(graphicEngine)->GetConstantBufferData<ObjectTransformGpuData>(transformMatrixBufferHandle);
	transformMatrixData->World = worldMatrix;
	transformMatrixData->WVP = QFE::MATH::Matrix4x4::Multiply(worldMatrix, viewProjectionMatrix);
}

bool QFE::GRAPHIC::CreateVertexBuffer(
	QFE::GRAPHIC::IGraphicEngine* graphicEngine, const std::vector<MeshVertexData>& vertexPositions,
	const std::string& meshName, QFE::GRAPHIC::DirectXResourceHandle& outVertexBufferHandle) {

	// 使用機能の取得
	QFE::GRAPHIC::DirectXDevice* directXDevice = ResolveD3D12GraphicEngine(graphicEngine)->GetDirectXDevice();
	QFE::GRAPHIC::DirectXResourceContainer* resourceContainer = ResolveD3D12GraphicEngine(graphicEngine)->GetDirectXResourceContainer();

	// バッファを作成
	QFE::GRAPHIC::DirectXResourceHandle handle =
		resourceContainer->CreateBuffer(directXDevice->GetDevice(), vertexPositions.size() * sizeof(MeshVertexData));
	QFE_LOG("VertexBuffer created for mesh: " + meshName + ", size: " + std::to_string(vertexPositions.size() * sizeof(MeshVertexData)) + " bytes");
	// バッファのストライドを設定
	resourceContainer->SetResourceStrideInBytes(handle, sizeof(MeshVertexData));
	// バッファにデータをコピー
	resourceContainer->MapResource(handle);
	MeshVertexData* mappedData = resourceContainer->GetMappedData<MeshVertexData>(handle);
	if (mappedData) {
		memcpy(mappedData, vertexPositions.data(), vertexPositions.size() * sizeof(MeshVertexData));
	}
	resourceContainer->SetResourceName(handle, ConvertString(meshName));
	resourceContainer->SetResourceStrideInBytes(handle, sizeof(MeshVertexData));

	// 成否の確認
	outVertexBufferHandle = handle;
	if (outVertexBufferHandle == QFE::GRAPHIC::DirectXResourceHandle::Invalid) {
		QFE_LOG("Failed to create vertex buffer for mesh: " + meshName);
		return false;
	}
	return true;
}

bool QFE::GRAPHIC::CreateIndexBuffer(QFE::GRAPHIC::IGraphicEngine* graphicEngine, const std::vector<uint32_t>& indices, const std::string& meshName, QFE::GRAPHIC::DirectXResourceHandle& outIndexBufferHandle) {
	// 使用機能の取得
	QFE::GRAPHIC::DirectXDevice* directXDevice = ResolveD3D12GraphicEngine(graphicEngine)->GetDirectXDevice();
	QFE::GRAPHIC::DirectXResourceContainer* resourceContainer = ResolveD3D12GraphicEngine(graphicEngine)->GetDirectXResourceContainer();

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

bool QFE::GRAPHIC::CreateBLAS(
	QFE::GRAPHIC::IGraphicEngine* graphicEngine, const std::vector<MeshVertexData>& vertices,
	const std::vector<uint32_t>& indices,
	const std::string& name, QFE::GRAPHIC::BLASHandle& outBLASHandle, bool allowUpdate) {

	// 使用機能の取得
	QFE::GRAPHIC::DirectXDevice* directXDevice = ResolveD3D12GraphicEngine(graphicEngine)->GetDirectXDevice();
	QFE::GRAPHIC::RaytracingAccelerationStructure* accelerationStructure = ResolveD3D12GraphicEngine(graphicEngine)->GetRaytracingAccelerationStructure();

	// vertexをVector3に変換してBLASを作成
	std::vector<QFE::MATH::Vector3> vertexPositions = GetModelVertexPositions(vertices.data(), vertices.size());
	outBLASHandle = accelerationStructure->CreateBLAS(
		directXDevice->GetDevice5(),
		ResolveD3D12GraphicEngine(graphicEngine)->GetDirectXCommandManager()->GetCommandList4(D3D12_COMMAND_LIST_TYPE_DIRECT),
		vertexPositions, indices, name, allowUpdate);
	// BLASの作成に失敗した場合はログを出力してfalseを返す
	if (outBLASHandle == QFE::GRAPHIC::BLASHandle::Invalid) {
		QFE_LOG("Failed to create BLAS for model: " + name);
		return false;
	}
	return true;
}

bool QFE::GRAPHIC::LoadTextureFromFile(QFE::GRAPHIC::IGraphicEngine* graphicEngine, const std::string& filePath, QFE::GRAPHIC::DirectXResourceHandle& outTextureHandle) {
	QFE::GRAPHIC::TextureLoader* textureLoader = ResolveD3D12GraphicEngine(graphicEngine)->GetTextureLoader();

	outTextureHandle = textureLoader->LoadTexture(filePath);
	if (outTextureHandle == QFE::GRAPHIC::DirectXResourceHandle::Invalid) {
		return false;
	}
	return true;
}

std::vector<QFE::MATH::Vector3> QFE::GRAPHIC::GetModelVertexPositions(const MeshVertexData* vertices, size_t vertexCount) {
	std::vector<QFE::MATH::Vector3> vertexPositions;
	vertexPositions.reserve(vertexCount);
	for (size_t i = 0; i < vertexCount; ++i) {
		const MeshVertexData& vertex = vertices[i];
		vertexPositions.push_back({ vertex.position.x, vertex.position.y, vertex.position.z });
	}
	return vertexPositions;
}

bool QFE::GRAPHIC::EnsureBufferCapacityAndUpload(
	QFE::GRAPHIC::IGraphicEngine* graphicEngine, QFE::GRAPHIC::DirectXResourceHandle& inOutHandle,
	const void* data, size_t byteSize, UINT elementStride, const std::string& name) {

	if (!graphicEngine) {
		return false;
	}
	auto* device = ResolveD3D12GraphicEngine(graphicEngine)->GetDirectXDevice();
	auto* rc = ResolveD3D12GraphicEngine(graphicEngine)->GetDirectXResourceContainer();

	// 空データではアップロードせず、既存のバッファは再利用可能なまま保持する。
	if (byteSize == 0) {
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

bool QFE::GRAPHIC::UploadGlobalMeshBuffers(
	QFE::GRAPHIC::IGraphicEngine* graphicEngine,
	const std::vector<RaytracingVertexAttribute>& globalVertexAttributes,
	const std::vector<uint32_t>& globalTriIndices, const std::vector<RaytracingInstanceMeta>& instanceMeta,
	QFE::GRAPHIC::DirectXResourceHandle& outUVHandle, QFE::GRAPHIC::DirectXResourceHandle& outTriHandle,
	QFE::GRAPHIC::DirectXResourceHandle& outInstanceMetaHandle) {

	if (!graphicEngine) return false;
	auto* device = ResolveD3D12GraphicEngine(graphicEngine)->GetDirectXDevice();
	auto* rc = ResolveD3D12GraphicEngine(graphicEngine)->GetDirectXResourceContainer();

	// 1) 頂点属性バッファ
	{
		size_t byteSize = globalVertexAttributes.size() * sizeof(RaytracingVertexAttribute);
		auto handle = rc->CreateBuffer(device->GetDevice(), byteSize);
		if (handle == QFE::GRAPHIC::DirectXResourceHandle::Invalid) return false;

		rc->MapResource(handle);
		RaytracingVertexAttribute* mapped = rc->GetMappedData<RaytracingVertexAttribute>(handle);
		if (mapped && !globalVertexAttributes.empty()) {
			memcpy(mapped, globalVertexAttributes.data(), byteSize);
		}
		rc->SetResourceName(handle, QFE::ConvertString(std::string("GlobalVertexAttributes")));
		rc->SetResourceStrideInBytes(handle, static_cast<UINT>(sizeof(RaytracingVertexAttribute)));

		// SRV 作成（Shader4ComponentMapping を明示的に設定）
		QFE::GRAPHIC::CreateViewInfo viewInfo{};
		viewInfo.viewType = QFE::GRAPHIC::ViewTypeFlags::ShaderResourceView;
		viewInfo.srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		viewInfo.srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		viewInfo.srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		viewInfo.srvDesc.Buffer.NumElements = static_cast<UINT>(globalVertexAttributes.size());
		viewInfo.srvDesc.Buffer.StructureByteStride = sizeof(RaytracingVertexAttribute);
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
		size_t byteSize = instanceMeta.size() * sizeof(RaytracingInstanceMeta);
		auto handle = rc->CreateBuffer(device->GetDevice(), byteSize);
		if (handle == QFE::GRAPHIC::DirectXResourceHandle::Invalid) return false;

		rc->MapResource(handle);
		RaytracingInstanceMeta* mapped = rc->GetMappedData<RaytracingInstanceMeta>(handle);
		if (mapped && !instanceMeta.empty()) {
			memcpy(mapped, instanceMeta.data(), byteSize);
		}
		rc->SetResourceName(handle, QFE::ConvertString(std::string("InstanceMeta")));
		rc->SetResourceStrideInBytes(handle, static_cast<UINT>(sizeof(RaytracingInstanceMeta)));

		QFE::GRAPHIC::CreateViewInfo viewInfo{};
		viewInfo.viewType = QFE::GRAPHIC::ViewTypeFlags::ShaderResourceView;
		viewInfo.srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		viewInfo.srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		viewInfo.srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		viewInfo.srvDesc.Buffer.NumElements = static_cast<UINT>(instanceMeta.size());
		viewInfo.srvDesc.Buffer.StructureByteStride = sizeof(RaytracingInstanceMeta);
		viewInfo.srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
		rc->CreateResourceView(handle, viewInfo);

		outInstanceMetaHandle = handle;
	}

	return true;
}

bool QFE::GRAPHIC::UpdateBLASInstanceBuffer(
	QFE::GRAPHIC::IGraphicEngine* graphicEngine, const std::vector<QFE::GRAPHIC::RaytracingInstance>& instances) {

	// 使用機能の取得
	QFE::GRAPHIC::DirectXCommandManager* commandManager = ResolveD3D12GraphicEngine(graphicEngine)->GetDirectXCommandManager();
	QFE::GRAPHIC::Fence* fence = ResolveD3D12GraphicEngine(graphicEngine)->GetFence();
	QFE::GRAPHIC::RaytracingAccelerationStructure* accelerationStructure = ResolveD3D12GraphicEngine(graphicEngine)->GetRaytracingAccelerationStructure();

	// TLASの更新
	accelerationStructure->UpdateTLAS(commandManager->GetCommandList4(D3D12_COMMAND_LIST_TYPE_DIRECT), instances);
	commandManager->ExecuteCommandList();
	fence->Signal(commandManager->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT));
	fence->Wait();
	commandManager->ResetCommandList();
	return true;
}
