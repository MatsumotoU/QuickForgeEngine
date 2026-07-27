#define NOMINMAX
#include <Windows.h>

#include "graphics/D3D12GraphicEngine.h"
#include "window/GameWindowManager.h"
#include "window/WindowsUtils.h"
#include "framework/graphic/D3D12GraphicFrameWork.h"
#include "framework/gui/D3D12GuiFrameWork.h"
#include "framework/script/WindowsScriptWorkFrame.h"
#include "gui/D3D12GuiManager.h"
#include "core/logger/MyDebugLog.h"
#include "core/process/ProcessUtil.h"
#include "script/ScriptFunctionList.h"
#include "script/ScriptInstance.h"
#include "components/AllComponent.h"

#include <imgui/imgui.h>

/// /// @brief Windowsアプリケーションのテスト
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	// デバッグログの初期化
	QFE::MyDebugLog::GetInstance()->Initialize();

	// ゲームウィンドウマネージャの初期化とウィンドウの追加
	std::unique_ptr<QFE::GameWindowManager> gameWindowManager = std::make_unique<QFE::GameWindowManager>();
	gameWindowManager->Initialize();
	gameWindowManager->AddWindow(1280, 720, "Test Window");
	HWND hWnd = gameWindowManager->GetWindow("Test Window");

	// ウィンドウのハンドルを取得してグラフィックエンジンを初期化
	std::unique_ptr<QFE::GRAPHIC::D3D12GraphicEngine> graphicEngine = 
		QFE::FRAMEWORK::CreateGraphicEngine(hWnd);
	// GUIマネージャの初期化
	std::unique_ptr<QFE::GUI::D3D12GuiManager> guiManager = 
		QFE::FRAMEWORK::CreateGuiManager(graphicEngine.get(), hWnd);

	// TestDll.dllをロードしてスクリプト関数の目録を取得
	std::unique_ptr<QFE::SCRIPT::WindowsScriptInstance> scriptInstance;
	std::wstring filePath;
	if (QFE::WINDOW::RequestGetFilePathFromUser(hWnd, L"GameLogic", L"*.dll", filePath)) {
		scriptInstance =QFE::FRAMEWORK::LoadWindowsScriptInstance(filePath, "GetManifest");
	}

	QFE::EntityManager entityManager;
	uint32_t testEntityId = entityManager.CreateEntity();
	entityManager.AddDefaultComponent<QFE::SCENE::TransformComponent>(testEntityId);
	QFE::SCENE::ScriptComponent& scriptComponent = entityManager.AddDefaultComponent<QFE::SCENE::ScriptComponent>(testEntityId);
	// 関数の登録
	uint32_t functionIndex = 0;
	scriptComponent.scriptFunctionName = scriptInstance->scripts[functionIndex].functionName;
	scriptComponent.scriptFunctionIndex = functionIndex;

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
			guiManager->PreDraw();

#ifdef USE_IMGUI
			ImGui::Begin("Test Window");
			QFE::MATH::EulerTransform& transform = entityManager.GetComponent<QFE::SCENE::TransformComponent>(testEntityId).transform;
			ImGui::Text("Position: (%.2f, %.2f, %.2f)", transform.translate.x, transform.translate.y, transform.translate.z);
			ImGui::Text("Rotation: (%.2f, %.2f, %.2f)", transform.rotate.x, transform.rotate.y, transform.rotate.z);
			ImGui::Text("Scale: (%.2f, %.2f, %.2f)", transform.scale.x, transform.scale.y, transform.scale.z);
			ImGui::End();
#endif // USE_IMGUI

			// スクリプト関数の実行
			entityManager.Each<QFE::SCENE::ScriptComponent>([&](uint32_t entityId, QFE::SCENE::ScriptComponent& scriptComp) {
				scriptInstance->scripts[scriptComp.scriptFunctionIndex].functionPtr(entityId, 0.016f, &entityManager);
				});

			
			graphicEngine->SetRenderTarget(QFE::GRAPHIC::RenderTargetHandle::SwapChain);
			guiManager->PostDraw();
			graphicEngine->PostDraw();
		}
	}

	// DLLをアンロード
	QFE::FRAMEWORK::UnloadWindowsScriptInstance(scriptInstance.get());

	guiManager->Shutdown();
	graphicEngine->Shutdown();
	gameWindowManager->Shutdown();
    return 0;
}
