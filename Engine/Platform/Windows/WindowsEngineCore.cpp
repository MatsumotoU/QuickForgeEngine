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
	// * ウィンドウマネージャー初期化 * //
	gameWindowManager = std::make_unique<GameWindowManager>();
	gameWindowManager->Initialize();
	gameWindowManager->AddWindow(windowWidth, windowHeight, "QuickForgeEngine");
	// * DirectX初期化 * //
	directXCommon_.Initialize(
		dynamic_cast<GameWindowManager*>(gameWindowManager.get())->GetWindow("QuickForgeEngine"), windowWidth, windowHeight);
	// * ImGuiの初期化 * //
	imguiFrameController_.Initialize(
		dynamic_cast<GameWindowManager*>(gameWindowManager.get())->GetWindow("QuickForgeEngine"),
		directXCommon_.GetCommandManager(D3D12_COMMAND_LIST_TYPE_DIRECT),
		directXCommon_.GetSrvDescriptorHeapAddressOf());
	ImGuiInitializer::Initialize(
		directXCommon_.GetDevice(),
		directXCommon_.GetBackBufferCount(),
		directXCommon_.GetSwapChainRtvDesc().Format,
		directXCommon_.GetSrvDescriptorHeapAddress(),
		directXCommon_.GetSrvDescriptorHeapAddress()->GetCPUDescriptorHandleForHeapStart(),
		directXCommon_.GetSrvDescriptorHeapAddress()->GetGPUDescriptorHandleForHeapStart());

	// * パイプライン管理クラス初期化 * //
	graphicPipelineManager_.Initialize(
		directXCommon_.GetDevice(),
		windowWidth,
		windowHeight,
		directXCommon_.GetDescriptorHeapManager()->GetDsvDescriptorHeap());

	// * オフスクリーンリソースマネージャー初期化 * //
	offScreenResourceManager_.Initialize(directXCommon_.GetDevice(), windowWidth, windowHeight);
	// オフスクリーンRTVヒープ割り当て & SRVヒープ割り当て
	for (uint32_t i = 0; i < offScreenResourceManager_.GetOffscreenCount(); i++) {
		// RTVヒープ割り当て
		DescriptorHandles rtvHandles = 
			directXCommon_.AssignRtvHeap(offScreenResourceManager_.GetOffscreenResource(i), &directXCommon_.GetSwapChainRtvDesc());
		offScreenResourceManager_.SetRtvHandle(rtvHandles.cpuHandle_, i);
		// SRVヒープ割り当て
		DescriptorHandles srvHandles =
			directXCommon_.AssignSrvHeap(offScreenResourceManager_.GetOffscreenResource(i), offScreenResourceManager_.GetOffscreenSrvDesc());
		offScreenResourceManager_.SetSrvHandle(srvHandles, i);
	}
	// * ポストプロセスマネージャー初期化 * //
	rendaringPostprocess_.Initialize(directXCommon_.GetDevice(), directXCommon_.GetCommandManager(D3D12_COMMAND_LIST_TYPE_DIRECT));
	rendaringPostprocess_.SetNormalPSO(graphicPipelineManager_.GetNormalPso());
	rendaringPostprocess_.SetColorCorrectionPSO(graphicPipelineManager_.GetColorCorrectionPso());
	rendaringPostprocess_.SetGrayScalePSO(graphicPipelineManager_.GetGrayScalePso());
	rendaringPostprocess_.SetVignettePSO(graphicPipelineManager_.GetVignettePso());
	rendaringPostprocess_.SetOffscreenResource(
		offScreenResourceManager_.GetOffscreenResource(0), offScreenResourceManager_.GetOffscreenResource(1));
	rendaringPostprocess_.SetOffscreenRtvHandle(
		offScreenResourceManager_.GetOffscreenRtvHandles(0), offScreenResourceManager_.GetOffscreenRtvHandles(1));
	rendaringPostprocess_.SetOffscreenSrvHandle(
		offScreenResourceManager_.GetOffscreenSrvHandles(0), offScreenResourceManager_.GetOffscreenSrvHandles(1));

	// * テクスチャマネージャー初期化 * //
	textureManager_.Initialize(&directXCommon_, directXCommon_.GetDescriptorHeapManager()->GetSrvDescriptorHeap());
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
	textureManager_.Finalize();
	imguiFrameController_.EndImGui();
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
	imguiFrameController_.BeginFrame();

	gameWindowManager->Draw();

	imguiFrameController_.EndFrame(directXCommon_.GetCurrentBackBufferCpuHandle());
	directXCommon_.PostDraw();

	textureManager_.ReleaseIntermediateResources();
}
