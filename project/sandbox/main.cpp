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

	std::string psDirName = "engine/resources/shaders/ps/";
	std::string vsDirName = "engine/resources/shaders/vs/";
	std::string rtDirName = "engine/resources/shaders/rt/";

	// UAVバッファの作成とルートリソースの設定
	QFE::GRAPHIC::DirectXResourceHandle uavBufferHandle = graphicEngine->CreateUAVBuffer(1280, 720, L"UAVBuffer");
	// レイトレーシングパイプラインステートオブジェクトの作成
	QFE::GRAPHIC::RTPSOHandle rtpsoHandle = graphicEngine->CreateRayTracingPipelineStateObject(
		rtDirName, "MiniRaytracing.hlsl");

	QFE::MATH::Transform transform;
	transform.scale = { 1.0f, 1.0f, 1.0f };
	QFE::MATH::Matrix4x4 transformMatrix = QFE::MATH::Matrix4x4::MakeAffineMatrix(transform);

	QFE::GRAPHIC::BLASHandle blasHandle = graphicEngine->CreateBLAS(
		{ { -1.0f, -1.0f, 0.0f }, { 1.0f, -1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
		"TriangleBLAS");
	QFE::GRAPHIC::BLASHandle inverseBlasHandle = graphicEngine->CreateBLAS(
		{ { -1.0f, 1.0f, 0.0f }, { 1.0f, 1.0f, 0.0f }, { 0.0f, -1.0f, 0.0f } },
		"InverseTriangleBLAS");
	QFE::GRAPHIC::BLASInstanceHandle blasInstanceHandle = graphicEngine->CreateBLASInstance(blasHandle, transformMatrix);
	QFE::GRAPHIC::BLASInstanceHandle inverseBlasInstanceHandle = graphicEngine->CreateBLASInstance(inverseBlasHandle, transformMatrix);

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

			graphicEngine->TestRayTracing(rtpsoHandle, uavBufferHandle);
			
			graphicEngine->SetRenderTarget(QFE::GRAPHIC::RenderTargetHandle::SwapChain);
			guiManager.PostDraw();
			graphicEngine->PostDraw();
		}
	}

	guiManager.Shutdown();
	graphicEngine->Shutdown();
	gameWindowManager->Shutdown();
    return 0;
}
