#define NOMINMAX
#include <Windows.h>

#include "window/GameWindowManager.h"
#include "window/WindowsUtils.h"
#include "framework/graphic/D3D12GraphicFrameWork.h"
#include "gui/D3D12GuiManager.h"
#include "core/loger/MyDebugLog.h"
#include "core/process/ProcessUtil.h"
#include "script/ScriptFunctionList.h"
#include "components/AllComponent.h"

#include <imgui/imgui.h>

struct TestScript {
	HMODULE gameDllHandle;
	std::vector<QFE::SCRIPT::ScriptFunctionInfo> scripts;
};

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
	TestScript testScript;
	std::wstring filePath;
	if (QFE::WINDOW::RequestGetFilePathFromUser(hWnd, L"GameLogic", L"*.dll", filePath)) {
		testScript.gameDllHandle = LoadLibraryW(filePath.c_str());
		assert(testScript.gameDllHandle != nullptr);

		FARPROC getManifestFunc = GetProcAddress(testScript.gameDllHandle, "GetManifest");
		assert(getManifestFunc != nullptr);

		GetManifestFunc GetManifest = reinterpret_cast<GetManifestFunc>(getManifestFunc);
		assert(GetManifest != nullptr);

		QFE::SCRIPT::ScriptFunctionInfo* functionArray = nullptr;
		// DLLから「ポインタ」と「個数」を受け取る
		size_t functionCount = GetManifest(&functionArray);

		// 安全にEXE側の管理するデータに詰め替える（これでDLLをアンロードしても安全！）
		testScript.scripts.reserve(functionCount);
		for (size_t i = 0; i < functionCount; ++i) {
			testScript.scripts.push_back(functionArray[i]);
		}
	}

	QFE::EntityManager entityManager;
	uint32_t testEntityId = entityManager.CreateEntity();
	entityManager.AddDefaultComponent<QFE::SCENE::TransformComponent>(testEntityId);
	QFE::SCENE::ScriptComponent& scriptComponent = entityManager.AddDefaultComponent<QFE::SCENE::ScriptComponent>(testEntityId);
	// 関数の登録
	uint32_t functionIndex = 0;
	scriptComponent.scriptFunctionName = testScript.scripts[functionIndex].functionName;
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

			ImGui::Begin("Test Window");
			
			QFE::MATH::Transform& transform = entityManager.GetComponent<QFE::SCENE::TransformComponent>(testEntityId).transform;
			ImGui::Text("Position: (%.2f, %.2f, %.2f)", transform.translate.x, transform.translate.y, transform.translate.z);
			ImGui::Text("Rotation: (%.2f, %.2f, %.2f)", transform.rotate.x, transform.rotate.y, transform.rotate.z);
			ImGui::Text("Scale: (%.2f, %.2f, %.2f)", transform.scale.x, transform.scale.y, transform.scale.z);
			ImGui::End();

			// スクリプト関数の実行
			entityManager.Each<QFE::SCENE::ScriptComponent>([&](uint32_t entityId, QFE::SCENE::ScriptComponent& scriptComp) {
				testScript.scripts[scriptComp.scriptFunctionIndex].functionPtr(entityId, 0.016f, &entityManager);
				});

			
			graphicEngine->SetRenderTarget(QFE::GRAPHIC::RenderTargetHandle::SwapChain);
			guiManager->PostDraw();
			graphicEngine->PostDraw();
		}
	}

	// DLLをアンロード
	FreeLibrary(testScript.gameDllHandle);

	guiManager->Shutdown();
	graphicEngine->Shutdown();
	gameWindowManager->Shutdown();
    return 0;
}
