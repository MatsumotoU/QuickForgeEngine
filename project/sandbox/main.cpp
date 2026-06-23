#define NOMINMAX
#include <Windows.h>

#include "window/GameWindowManager.h"
#include "graphics/D3D12GraphicEngine.h"
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

	// UAVバッファの作成とルートリソースの設定
	QFE::GRAPHIC::DirectXResourceHandle uavBufferHandle = graphicEngine->CreateUAVBuffer(1280, 720);

	SphireForGPU sphereData;
	sphereData.center = { 0.0f, 0.0f, 0.5f };
	sphereData.radius = 1.0f;
	QFE::GRAPHIC::DirectXResourceHandle constantBufferHandle = 
		graphicEngine->CreateConstantBuffer<SphireForGPU>(sphereData, "TestConstantBuffer");

	QFE::GRAPHIC::ComputePSOHandle computePSOHandle = 
		graphicEngine->CreateComputePipelineStateObject("engine/resources/shaders/cs/", "TestCompute.hlsl");

	float time = 0.0f;

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

			time += 0.016f; // 仮の時間の更新
			SphireForGPU* sphereDataPtr = graphicEngine->GetConstantBufferData<SphireForGPU>(constantBufferHandle);
			sphereDataPtr->center.x = sinf(time) * 3.0f;
			sphereDataPtr->center.z = cosf(time) * 3.0f;
			sphereDataPtr->center.y = cosf(time) + 1.0f;

			graphicEngine->TestCompute(computePSOHandle, uavBufferHandle, constantBufferHandle);

			graphicEngine->PostDraw();
		}
	}

	graphicEngine->Shutdown();
	gameWindowManager->Shutdown();
    return 0;
}
