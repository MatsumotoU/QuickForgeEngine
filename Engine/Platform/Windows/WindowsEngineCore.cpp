#include "WindowsEngineCore.h"

#include "AppUtility/DebugTool/ImGui/Initializer/ImGuiInitializer.h"
#ifdef _DEBUG
#include "AppUtility/DebugTool/DebugLog/MyDebugLog.h"
#endif // _DEBUG

namespace {
	uint32_t windowWidth = 1280;
	uint32_t windowHeight = 720;
}

WindowsEngineCore::WindowsEngineCore(HINSTANCE& hInstance, LPSTR& lpCmdLine) 
	:debugCore_(lpCmdLine),hInstance_(hInstance),lpCmdLine_(lpCmdLine){}

void WindowsEngineCore::Initialize() {
	gameWindowManager = std::make_unique<GameWindowManager>();
	gameWindowManager->Initialize();
	gameWindowManager->AddWindow(windowWidth, windowHeight, "QuickForgeEngine");

	directXCommon_.Initialize(
		dynamic_cast<GameWindowManager*>(gameWindowManager.get())->GetWindow("QuickForgeEngine"), windowWidth, windowHeight);

	imguiFrameController_.Initialize(
		dynamic_cast<GameWindowManager*>(gameWindowManager.get())->GetWindow("QuickForgeEngine"),
		directXCommon_.GetCommandManager(D3D12_COMMAND_LIST_TYPE_DIRECT),
		directXCommon_.GetSrvDescriptorHeapAddressOf());
	ImGuiInitializer::Initialize(
		directXCommon_.GetDevice(),
		2,
		DXGI_FORMAT_R8G8B8A8_UNORM,
		directXCommon_.GetSrvDescriptorHeapAddress(),
		directXCommon_.GetSrvDescriptorHeapAddress()->GetCPUDescriptorHandleForHeapStart(),
		directXCommon_.GetSrvDescriptorHeapAddress()->GetGPUDescriptorHandleForHeapStart());
}

void WindowsEngineCore::MainLoop() {
	while (gameWindowManager->IsWindowActive())
	{
		// アプリケーション安全終了処理
		MSG msg;
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT) {
				break;
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);

		} else {
			Update();
			Draw();
		}
	}
}

void WindowsEngineCore::Shutdown() {
	directXCommon_.Shutdown();
	gameWindowManager->Shutdown();
#ifdef _DEBUG
	DebugLog("FinalizeEngine");
#endif // _DEBUG
}

void WindowsEngineCore::Update() {
	gameWindowManager->Update();
}

void WindowsEngineCore::Draw() {
	directXCommon_.PreDraw();
	gameWindowManager->Draw();
	directXCommon_.PostDraw();
}
