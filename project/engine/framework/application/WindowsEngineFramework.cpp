#include "WindowsEngineFramework.h"
#include "EngineDefines.h"

// 機能をunique_ptrをつかって管理するために必要なヘッダーファイル
#include "window/GameWindowManager.h"
#include "graphics/D3D12GraphicEngine.h"
#include "gui/D3D12GuiManager.h"
#include "input/InputInterface.h"
#include "scene/SceneManager.h"
#include "script/ScriptInstance.h"

bool QFE::FRAMEWORK::CreateWindowsQuickForgeEngineSystems(
	HINSTANCE hInstance, 
	const std::string& mainWindowName, uint32_t mainWindowWidth, uint32_t mainWindowHeight,
	WindowsQuickForgeEngineSystems& outSystems) {

	// デバッグログの初期化
	QFE::MyDebugLog::GetInstance()->Initialize();

	// ゲームウィンドウマネージャの初期化とウィンドウの追加
	outSystems.windowManager = 
		QFE::FRAMEWORK::CreateWindowManager(mainWindowName, mainWindowWidth, mainWindowHeight);
	// ウィンドウマネージャの初期化に失敗した場合はエラーを報告して終了
	if(outSystems.windowManager == nullptr) {
		QFE_REPORT_SYSTEM_ERROR("Failed to create GameWindowManager.",QFE::SystemError::Abort);
		return false;
	}
	// メインウィンドウのハンドルを取得
	HWND mainWindow = QFE::FRAMEWORK::GetWindowHandle(outSystems.windowManager.get(), mainWindowName);

	// ウィンドウのハンドルを取得してグラフィックエンジンを初期化
	outSystems.graphicEngine =
		QFE::FRAMEWORK::CreateGraphicEngine(mainWindow);
	// グラフィックエンジンの初期化に失敗した場合はエラーを報告して終了
	if(outSystems.graphicEngine == nullptr) {
		QFE_REPORT_SYSTEM_ERROR("Failed to create D3D12GraphicEngine.",QFE::SystemError::Abort);
		return false;
	}

	outSystems.guiManager = QFE::FRAMEWORK::CreateGuiManager(outSystems.graphicEngine.get(), mainWindow);
	if (outSystems.guiManager == nullptr) {
		QFE_REPORT_SYSTEM_ERROR("Failed to create D3D12GuiManager.", QFE::SystemError::Abort);
		return false;
	}

	// InputInterfaceの初期化
	outSystems.inputInterface = QFE::FRAMEWORK::CreateInputInterface(mainWindow, hInstance);
	// InputInterfaceの初期化に失敗した場合はエラーを報告して終了
	if (outSystems.inputInterface == nullptr) {
		QFE_REPORT_SYSTEM_ERROR("Failed to create InputInterface.", QFE::SystemError::Abort);
		return false;
	}

	// FPSカウンターの初期化
	outSystems.fpsCounter = std::make_unique<QFE::FPSCounter>();
	outSystems.fpsCounter->Reset();

	// シーンマネージャの初期化
	outSystems.sceneManager = std::make_unique<QFE::SCENE::SceneManager>();
	outSystems.sceneManager->Initialize();

	return true;
}

bool QFE::FRAMEWORK::ProcessWindowsApplicationMessage() {
	MSG msg{};
	if (!PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
		return true;
	}
	if (msg.message == WM_QUIT) {
		return false;
	}
	TranslateMessage(&msg);
	DispatchMessage(&msg);
	return true;
}

bool QFE::FRAMEWORK::BeginWindowsEngineFrame(WindowsQuickForgeEngineSystems& systems) {
	if (!systems.fpsCounter || !systems.inputInterface) {
		return false;
	}
	systems.fpsCounter->FrameStart();
	systems.inputInterface->Update();
	return true;
}

void QFE::FRAMEWORK::EndWindowsEngineFrame(WindowsQuickForgeEngineSystems& systems) {
	if (systems.guiManager) systems.guiManager->PostDraw();
	if (systems.graphicEngine) systems.graphicEngine->PostDraw();
	if (systems.sceneManager) systems.sceneManager->EndFrame();
	if (systems.fpsCounter) systems.fpsCounter->FrameEnd();
	if (systems.inputInterface) systems.inputInterface->EndFrame();
}

void QFE::FRAMEWORK::ShutdownWindowsQuickForgeEngineSystems(WindowsQuickForgeEngineSystems& systems) {
	if (systems.sceneManager) systems.sceneManager->Shutdown();
	if (systems.guiManager) systems.guiManager->Shutdown();
	if (systems.graphicEngine) systems.graphicEngine->Shutdown();
	if (systems.windowManager) systems.windowManager->Shutdown();
}
