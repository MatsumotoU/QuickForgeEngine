#include "D3D12GraphicFrameWork.h"

#include "graphics/D3D12GraphicEngine.h"
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

bool QFE::FRAMEWORK::CreateShaderPair(
	QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine, const std::string& vsDirName, const std::string& psDirName, const std::string& vsFileName, const std::string& psFileName, QFE::GRAPHIC::ShaderPairHandle& outShaderPairHandle) {
	// シェーダーペア生成の設定
	QFE::GRAPHIC::ShaderPairElement shaderPairElement;
	shaderPairElement.vsDirName = vsDirName;
	shaderPairElement.psDirName = psDirName;
	shaderPairElement.vsFileName = vsFileName;
	shaderPairElement.psFileName = psFileName;

	// シェーダーペアを生成
	outShaderPairHandle = graphicEngine->CreateShaderPair(shaderPairElement);

	// シェーダーペアの生成に成功したかを返す
	return outShaderPairHandle != QFE::GRAPHIC::ShaderPairHandle::Invalid;
}

bool QFE::FRAMEWORK::CreateObject3dGBufferRootResources(
	QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine, QFE::GRAPHIC::DirectXResourceHandle* rootResources, size_t rootResourcesSize) {

	// ルートリソースの数を確認
	if (rootResourcesSize < 3) {
		QFE_LOG("rootResourcesSize is too small. It should be at least 3.");
		return false;
	}

	// リソースアロケータを取得
	QFE::GRAPHIC::DirectXResourceAllocator* resourceAllocator = graphicEngine->GetResourceAllocator();

	// EulerTransform
	TransformationMatrix transformMatrix;
	transformMatrix.World = QFE::MATH::Matrix4x4::MakeIndentity4x4();
	transformMatrix.WVP = QFE::MATH::Matrix4x4::MakeIndentity4x4();
	QFE::GRAPHIC::DirectXResourceHandle transformMatrixBufferHandle =
		resourceAllocator->AllocateConstantBuffer<TransformationMatrix>("TransformMatrixBuffer");
	// Material
	Material material;
	material.enableLighting = false;
	material.color = { 1.0f, 1.0f, 1.0f, 1.0f };
	QFE::GRAPHIC::DirectXResourceHandle materialBufferHandle =
		resourceAllocator->AllocateConstantBuffer<Material>("MaterialBuffer");
	//Texture
	QFE::GRAPHIC::DirectXResourceHandle textureHandle =
		graphicEngine->GetBuiltInTextureHandle(QFE::GRAPHIC::BuiltInTextureType::DummyWhite1x1Texture);

	// ルートリソースに設定
	rootResources[0] = transformMatrixBufferHandle;
	rootResources[1] = materialBufferHandle;
	rootResources[2] = textureHandle;

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

	outVertexBufferHandle = graphicEngine->CreateVertexBuffer(vertexPositions, meshName);
	if(outVertexBufferHandle == QFE::GRAPHIC::DirectXResourceHandle::Invalid) {
		QFE_LOG("Failed to create vertex buffer for mesh: " + meshName);
		return false;
	}
	return true;
}

bool QFE::FRAMEWORK::CreateBLAS(
	QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine, const std::vector<VertexData>& vertices,
	const std::string& name, QFE::GRAPHIC::BLASHandle& outBLASHandle) {

	std::vector<QFE::MATH::Vector3> vertexPositions = GetModelVertexPositions(vertices.data(), vertices.size());
	outBLASHandle = graphicEngine->CreateBLAS(vertexPositions, name);

	if(outBLASHandle == QFE::GRAPHIC::BLASHandle::Invalid) {
		QFE_LOG("Failed to create BLAS for model: " + name);
		return false;
	}
	return true;
}
