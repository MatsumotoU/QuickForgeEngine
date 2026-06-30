#define NOMINMAX
#include <Windows.h>

#include "window/GameWindowManager.h"
#include "graphics/D3D12GraphicEngine.h"
#include "camera/CameraManager.h"
#include "core/loger/MyDebugLog.h"

#include "assetfactory/model/AssimpModelLoader.h"

#include "core/math/transform/Transform.h"

/// /// @brief Windowsアプリケーションのテスト
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	// デバッグログの初期化
	QFE::MyDebugLog::GetInstance()->Initialize();

	// ゲームウィンドウマネージャの初期化とウィンドウの追加
	std::unique_ptr<QFE::GameWindowManager> gameWindowManager = std::make_unique<QFE::GameWindowManager>();
	gameWindowManager->Initialize();
	gameWindowManager->AddWindow(1280, 720, "Test Window");

	// ウィンドウのハンドルを取得してグラフィックエンジンを初期化
	std::unique_ptr<QFE::GRAPHIC::D3D12GraphicEngine> graphicEngine = 
		std::make_unique<QFE::GRAPHIC::D3D12GraphicEngine>(gameWindowManager->GetWindow("Test Window"));
	graphicEngine->Initialize();

	QFE::CAMERA::CameraManager cameraManager;
	cameraManager.Initialize();
	QFE::CAMERA::CameraHandle cameraHandle = cameraManager.CreateCamera(0.0f, 1280.0f, 0.0f, 720.0f, 0.1f, 100.0f, 45.0f);

	// ビルトインのパイプラインステートオブジェクトを取得
	QFE::GRAPHIC::PSOHandle psoHandle = graphicEngine->GetBuiltInPipelineStateObject(
		QFE::GRAPHIC::BuiltInShaderPair::Object3D, QFE::GRAPHIC::BlendMode::kBlendModeNormal,
		QFE::GRAPHIC::RasterizerType::Default, QFE::GRAPHIC::DepthStencilDescType::Default);

	QFE::MATH::Transform objTransform;
	QFE::MATH::Transform cameraTransform;
	cameraTransform.translate.z = -5.0f;

	// Transform
	TransformationMatrix transformMatrix;
	transformMatrix.World = QFE::MATH::Matrix4x4::MakeAffineMatrix(objTransform);
	transformMatrix.WVP = QFE::MATH::Matrix4x4::Multiply(transformMatrix.World, cameraManager.GetViewProjectionMatrix(cameraHandle, cameraTransform, QFE::CAMERA::CameraType::Perspective));
	QFE::GRAPHIC::DirectXResourceHandle transformMatrixBufferHandle = graphicEngine->CreateConstantBuffer<TransformationMatrix>(
		transformMatrix, "TransformMatrixBuffer");
	// Material
	Material material;
	material.color = { 1.0f, 0.0f, 0.0f, 1.0f };
	QFE::GRAPHIC::DirectXResourceHandle materialBufferHandle = graphicEngine->CreateConstantBuffer<Material>(
		material, "MaterialBuffer");
	// DirectionalLight
	DirectionalLight directionalLight;
	directionalLight.color = { 1.0f, 1.0f, 1.0f, 1.0f };
	directionalLight.direction = { 0.0f, -1.0f, 0.0f };
	directionalLight.intensity = 1.0f;
	QFE::GRAPHIC::DirectXResourceHandle directionalLightBufferHandle = graphicEngine->CreateConstantBuffer<DirectionalLight>(
		directionalLight, "DirectionalLightBuffer");
	//CameraForGPU;
	CameraForGPU cameraForGPU;
	cameraForGPU.cameraPosition = cameraTransform.translate;
	QFE::GRAPHIC::DirectXResourceHandle cameraForGPUBufferHandle = graphicEngine->CreateConstantBuffer<CameraForGPU>(
		cameraForGPU, "CameraForGPUBuffer");
	//Texture2D<float32_t4> gTexture : register(t0);
	QFE::GRAPHIC::DirectXResourceHandle textureHandle = 
		graphicEngine->GetBuiltInTextureHandle(QFE::GRAPHIC::BuiltInTextureType::DummyWhite1x1Texture);
	//TextureCube<float32_t4> gCubeTexture : register(t1);
	QFE::GRAPHIC::DirectXResourceHandle cubeTextureHandle = 
		graphicEngine->GetBuiltInTextureHandle(QFE::GRAPHIC::BuiltInTextureType::DummyBlackCubeMap);

	// Vertexバッファの作成とモデルデータの読み込み
	QFE::ASSET::AssimpModelLoader modelLoader;
	modelLoader.Initialize();
	QFE::ASSET::ModelData& modelData = modelLoader.LoadModel("resources/0.obj");
	QFE::GRAPHIC::DirectXResourceHandle vertexBufferHandle = graphicEngine->CreateVertexBuffer(modelData.meshes[0].vertices.GetInternalVector(), "VertexBuffer");

	std::vector<QFE::GRAPHIC::DirectXResourceHandle> rootResources = {
		transformMatrixBufferHandle,
		materialBufferHandle,
		directionalLightBufferHandle,
		cameraForGPUBufferHandle,
		textureHandle,
		cubeTextureHandle
	};

	QFE::GRAPHIC::ViewPortHandle viewportHandle = graphicEngine->CreateViewPort(1280, 720);
	QFE::GRAPHIC::ScissorRectHandle scissorRectHandle = graphicEngine->CreateScissorRect(0, 0, 1280, 720);

	// メインループ
	while (gameWindowManager->IsWindowActive()) {
		MSG msg;
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			// WM_QUITメッセージが来たらループを抜ける
			if (msg.message == WM_QUIT) {
				break;
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);

		} else {
			graphicEngine->PreDraw();

			graphicEngine->TestDraw(psoHandle, viewportHandle, scissorRectHandle,vertexBufferHandle, rootResources);

			graphicEngine->PostDraw();
		}
	}

	graphicEngine->Shutdown();
	gameWindowManager->Shutdown();
    return 0;
}
