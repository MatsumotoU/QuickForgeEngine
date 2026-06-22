#define NOMINMAX
#include <Windows.h>

#include "window/GameWindowManager.h"
#include "graphics/D3D12GraphicEngine.h"
#include "core/loger/MyDebugLog.h"

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

	QFE::GRAPHIC::ViewPortHandle viewPortHandle = graphicEngine->CreateViewPort(1280, 720);
	QFE::GRAPHIC::ScissorRectHandle scissorRectHandle = graphicEngine->CreateScissorRect(0, 0, 1280, 720);

	//QFE::GRAPHIC::DirectXResourceHandle vertexBufferHandle = graphicEngine->CreateVertexBuffer(vertexData, "Triangle");

	QFE::GRAPHIC::PSOHandle psoHandle = graphicEngine->GetBuiltInPipelineStateObject(
		QFE::GRAPHIC::BuiltInShaderPair::Object3D, QFE::GRAPHIC::BlendMode::kBlendModeNone,
		QFE::GRAPHIC::RasterizerType::Default, QFE::GRAPHIC::DepthStencilDescType::None);

	std::vector<QFE::GRAPHIC::DirectXResourceHandle> rootResources;
	/*QFE::MATH::Transform transform = {};
	transform.translate = { 0.0f, 0.0f, 0.0f };
	transform.rotate = { 0.0f, 0.0f, 0.0f };
	transform.scale = { 1.0f, 1.0f, 1.0f };
	TransformationMatrix transformMatrix = {};
	transformMatrix.World = QFE::MATH::Matrix4x4::MakeAffineMatrix(transform);
	transformMatrix.WVP = QFE::MATH::Matrix4x4::MakeIndentity4x4();
	QFE::GRAPHIC::DirectXResourceHandle constantBufferHandle = 
		graphicEngine->CreateConstantBuffer<TransformationMatrix>(transformMatrix, "TransformMatrix");
	rootResources.push_back(constantBufferHandle);*/

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
			//graphicEngine->GetConstantBufferData<TransformationMatrix>(constantBufferHandle)->World = QFE::MATH::Matrix4x4::MakeAffineMatrix(transform);
			//graphicEngine->TestDraw(psoHandle,viewPortHandle, scissorRectHandle, vertexBufferHandle, rootResources);

			graphicEngine->PostDraw();
		}
	}

	graphicEngine->Shutdown();
	gameWindowManager->Shutdown();
    return 0;
}
