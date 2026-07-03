#include "D3D12GraphicFrameWork.h"
#include "core/math/transform/Transform.h"

std::unique_ptr<QFE::GRAPHIC::D3D12GraphicEngine> QFE::FRAMEWORK::CreateGraphicEngine(HWND hwnd) {
	// ウィンドウのハンドルを取得してグラフィックエンジンを初期化
	std::unique_ptr<QFE::GRAPHIC::D3D12GraphicEngine> graphicEngine =
		std::make_unique<QFE::GRAPHIC::D3D12GraphicEngine>(hwnd);
	graphicEngine->Initialize();
	return graphicEngine;
}

std::unique_ptr<QFE::GUI::D3D12GuiManager> QFE::FRAMEWORK::CreateGuiManager(QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine, HWND hwnd) {
	// GUIマネージャの初期化
	QFE::GUI::D3D12GuiManager guiManager;
	QFE::GUI::D3D12GuiManagerInitDesc guiInitDesc;
	guiInitDesc.device = graphicEngine->GetDevice();
	guiInitDesc.commandList = graphicEngine->GetCommandList(D3D12_COMMAND_LIST_TYPE_DIRECT);
	guiInitDesc.bufferCount = graphicEngine->GetSwapChainBufferCount();
	guiInitDesc.rtvFormat = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	guiInitDesc.srvHeap = graphicEngine->GetSRVDescriptorHeap();
	QFE::GRAPHIC::DescriptorHandles srvHandles = graphicEngine->CreateExternalSRVDescriptor();
	guiInitDesc.cpuHandle = srvHandles.cpuHandle_;
	guiInitDesc.gpuHandle = srvHandles.gpuHandle_;
	guiInitDesc.hwnd = hwnd;
	guiManager.Initialize(guiInitDesc);

	return std::make_unique<QFE::GUI::D3D12GuiManager>(std::move(guiManager));
}

QFE::GRAPHIC::ShaderPairHandle QFE::FRAMEWORK::CreateObject3dGBufferShaderPair(
	QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine, const std::string& vsDirName,const std::string& psDirName,
	const std::string& vsFileName, const std::string& psFileName) {

	// シェーダーペア生成の設定
	QFE::GRAPHIC::ShaderPairElement shaderPairElement;
	shaderPairElement.vsDirName = vsDirName;
	shaderPairElement.psDirName = psDirName;
	shaderPairElement.vsFileName = vsFileName;
	shaderPairElement.psFileName = psFileName;
	// シェーダーペアを生成
	return graphicEngine->CreateShaderPair(shaderPairElement);
}

void QFE::FRAMEWORK::CreateObject3dGBufferRootResources(
	QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine, std::vector<QFE::GRAPHIC::DirectXResourceHandle>& rootResources) {

	// Transform
	TransformationMatrix transformMatrix;
	transformMatrix.World = QFE::MATH::Matrix4x4::MakeIndentity4x4();
	transformMatrix.WVP = QFE::MATH::Matrix4x4::MakeIndentity4x4();
	QFE::GRAPHIC::DirectXResourceHandle transformMatrixBufferHandle = graphicEngine->CreateConstantBuffer<TransformationMatrix>(
		transformMatrix, "TransformMatrixBuffer");
	// Material
	Material material;
	material.enableLighting = false;
	material.color = { 1.0f, 1.0f, 1.0f, 1.0f };
	QFE::GRAPHIC::DirectXResourceHandle materialBufferHandle = graphicEngine->CreateConstantBuffer<Material>(
		material, "MaterialBuffer");
	//Texture
	QFE::GRAPHIC::DirectXResourceHandle textureHandle =
		graphicEngine->GetBuiltInTextureHandle(QFE::GRAPHIC::BuiltInTextureType::DummyWhite1x1Texture);

	// ルートリソースをクリアして追加
	rootResources.clear();
	rootResources.push_back(transformMatrixBufferHandle);
	rootResources.push_back(materialBufferHandle);
	rootResources.push_back(textureHandle);
}

void QFE::FRAMEWORK::UpdateObject3dWVPMatrix(
	QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine, QFE::GRAPHIC::DirectXResourceHandle transformMatrixBufferHandle,
	const QFE::MATH::Transform& transform, const QFE::MATH::Matrix4x4& viewProjectionMatrix) {

	TransformationMatrix* transformMatrixData = 
		graphicEngine->GetConstantBufferData<TransformationMatrix>(transformMatrixBufferHandle);
	transformMatrixData->World = QFE::MATH::Matrix4x4::MakeAffineMatrix(transform);
	transformMatrixData->WVP = QFE::MATH::Matrix4x4::Multiply(transformMatrixData->World, viewProjectionMatrix);
}
