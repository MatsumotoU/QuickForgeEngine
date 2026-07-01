#define NOMINMAX
#include <Windows.h>

#include "window/GameWindowManager.h"
#include "graphics/D3D12GraphicEngine.h"
#include "gui/D3D12GuiManager.h"
#include "camera/CameraManager.h"
#include "core/loger/MyDebugLog.h"

#include "assetfactory/model/AssimpModelLoader.h"

#include "core/math/transform/Transform.h"

#include <imgui/imgui.h>

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
	guiInitDesc.hwnd = gameWindowManager->GetWindow("Test Window");
	guiManager.Initialize(guiInitDesc);

	// カメラマネージャの初期化とカメラの作成
	QFE::CAMERA::CameraManager cameraManager;
	cameraManager.Initialize();
	QFE::CAMERA::CameraHandle cameraHandle = cameraManager.CreateCamera(0.0f, 1280.0f, 0.0f, 720.0f, 0.1f, 100.0f, 45.0f);

	// ビルトインのパイプラインステートオブジェクトを取得
	QFE::GRAPHIC::PSOHandle psoHandle = graphicEngine->GetBuiltInPipelineStateObject(
		QFE::GRAPHIC::BuiltInShaderPair::Object3D, QFE::GRAPHIC::BlendMode::kBlendModeNormal,
		QFE::GRAPHIC::RasterizerType::Default, QFE::GRAPHIC::DepthStencilDescType::Default);

	QFE::MATH::Transform objTransform;
	QFE::MATH::Transform cameraTransform;
	cameraTransform.translate.z = -2.0f;

	// Transform
	TransformationMatrix transformMatrix;
	transformMatrix.World = QFE::MATH::Matrix4x4::MakeAffineMatrix(objTransform);
	transformMatrix.WVP = QFE::MATH::Matrix4x4::Multiply(transformMatrix.World, cameraManager.GetViewProjectionMatrix(cameraHandle, cameraTransform, QFE::CAMERA::CameraType::Perspective));
	QFE::GRAPHIC::DirectXResourceHandle transformMatrixBufferHandle = graphicEngine->CreateConstantBuffer<TransformationMatrix>(
		transformMatrix, "TransformMatrixBuffer");
	// Material
	Material material;
	material.enableLighting = false;
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
	//Texture
	QFE::GRAPHIC::DirectXResourceHandle textureHandle = 
		graphicEngine->GetBuiltInTextureHandle(QFE::GRAPHIC::BuiltInTextureType::DummyWhite1x1Texture);
	//gCubeTexture
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
			guiManager.PreDraw();

			objTransform.rotate.y += 0.01f;

#ifdef USE_IMGUI
			ImGui::Begin("Test Window");
			ImGui::Text("Hello, world!");
			ImGui::SliderFloat3("Object Position", &objTransform.translate.x, -10.0f, 10.0f);
			ImGui::End();
#endif
			TransformationMatrix* transformMatrixData = graphicEngine->GetConstantBufferData<TransformationMatrix>(transformMatrixBufferHandle);
			transformMatrixData->World = QFE::MATH::Matrix4x4::MakeAffineMatrix(objTransform);
			transformMatrixData->WVP = QFE::MATH::Matrix4x4::Multiply(transformMatrixData->World, cameraManager.GetViewProjectionMatrix(cameraHandle, cameraTransform, QFE::CAMERA::CameraType::Perspective));

			graphicEngine->TestDraw(psoHandle, viewportHandle, scissorRectHandle,vertexBufferHandle, rootResources);
			
			guiManager.PostDraw();
			graphicEngine->PostDraw();
		}
	}

	guiManager.Shutdown();
	graphicEngine->Shutdown();
	gameWindowManager->Shutdown();
    return 0;
}
