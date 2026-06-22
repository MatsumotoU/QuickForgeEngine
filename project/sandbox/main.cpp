#define NOMINMAX
#include <Windows.h>

#include "window/GameWindowManager.h"
#include "graphics/D3D12GraphicEngine.h"
#include "core/loger/MyDebugLog.h"

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

	QFE::GRAPHIC::ViewPortHandle viewPortHandle = graphicEngine->CreateViewPort(1280, 720);
	QFE::GRAPHIC::ScissorRectHandle scissorRectHandle = graphicEngine->CreateScissorRect(0, 0, 1280, 720);
	std::vector<VertexData> vertexData;
	VertexData v1;
	v1.position = { 0.0f, 0.5f, 0.0f, 1.0f };
	VertexData v2;
	v2.position = { 0.5f, -0.5f, 0.0f, 1.0f };
	VertexData v3;
	v3.position = { -0.5f, -0.5f, 0.0f, 1.0f };
	vertexData.push_back(v1);
	vertexData.push_back(v2);
	vertexData.push_back(v3);

	QFE::GRAPHIC::DirectXResourceHandle vertexBufferHandle = graphicEngine->CreateVertexBuffer(vertexData, "Triangle");

	std::string vsDir = "engine/resources/shaders/vs/";
	std::string psDir = "engine/resources/shaders/ps/";
	QFE::GRAPHIC::ShaderPairHandle shaderPairHandle = graphicEngine->CreateShaderPair({ vsDir, "Object2d.VS.hlsl", psDir, "ObjectMini.hlsl" });

	QFE::GRAPHIC::PSOHandle psoHandle = graphicEngine->CreatePipelineStateObject(
		shaderPairHandle, QFE::GRAPHIC::BlendMode::kBlendModeNone, QFE::GRAPHIC::RasterizerType::Default, QFE::GRAPHIC::DepthStencilDescType::None);


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
			// 描画処理
			//graphicEngine->TestDraw(psoHandle,viewPortHandle, scissorRectHandle, vertexBufferHandle);

			graphicEngine->PostDraw();
		}
	}

	graphicEngine->Shutdown();
	gameWindowManager->Shutdown();
    return 0;
}
