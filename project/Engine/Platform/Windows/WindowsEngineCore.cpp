#include "WindowsEngineCore.h"
#include "Editor/OnWindowsEditor.h"

#include <thread>
#include "AppUtility/DebugTool/ImGui/Initializer/ImGuiInitializer.h"
#ifdef _DEBUG
#include "AppUtility/DebugTool/DebugLog/MyDebugLog.h"
#include "Renderer/ModelRenderer.h"
#endif // _DEBUG
#include "Core/EngineGlobalValue.h"
#include "AppUtility/FileSystems/FileUtility.h"

namespace {
	uint32_t windowWidth = 1280;
	uint32_t windowHeight = 720;
}

WindowsEngineCore::WindowsEngineCore(HINSTANCE& hInstance, LPSTR& lpCmdLine) 
	:debugCore_(lpCmdLine),hInstance_(hInstance),lpCmdLine_(lpCmdLine){
}

void WindowsEngineCore::Initialize() {
	QFE::EngineGlobalValue::windowWidth = windowWidth;
	QFE::EngineGlobalValue::windowHeight = windowHeight;

	// * ウィンドウマネージャー初期化 * //
	gameWindowManager = std::make_unique<GameWindowManager>();
	gameWindowManager->Initialize();
	gameWindowManager->AddWindow(windowWidth, windowHeight, "QuickForgeEngine");
	// * DirectX初期化 * //
	directXCommon_ = DirectXCommon::GetInstance();
	directXCommon_->Initialize(
		dynamic_cast<GameWindowManager*>(gameWindowManager.get())->GetWindow("QuickForgeEngine"), windowWidth, windowHeight);
	// * ImGuiの初期化 * //
	imguiFrameController_.Initialize(
		dynamic_cast<GameWindowManager*>(gameWindowManager.get())->GetWindow("QuickForgeEngine"),
		directXCommon_->GetCommandManager(D3D12_COMMAND_LIST_TYPE_DIRECT),
		directXCommon_->GetSrvDescriptorHeapAddressOf());
	ImGuiInitializer::Initialize(
		directXCommon_->GetDevice(),
		directXCommon_->GetBackBufferCount(),
		directXCommon_->GetSwapChainRtvDesc().Format,
		directXCommon_->GetSrvDescriptorHeapAddress(),
		directXCommon_->GetSrvDescriptorHeapAddress()->GetCPUDescriptorHandleForHeapStart(),
		directXCommon_->GetSrvDescriptorHeapAddress()->GetGPUDescriptorHandleForHeapStart());

	// * パイプライン管理クラス初期化 * //
	graphicPipelineManager_ = GraphicPipelineManager::GetInstance();
	graphicPipelineManager_->Initialize(directXCommon_->GetDevice());

	// * オフスクリーンリソースマネージャー初期化 * //
	offScreenResourceManager_.Initialize(directXCommon_->GetDevice(), windowWidth, windowHeight);
	// オフスクリーンRTVヒープ割り当て & SRVヒープ割り当て
	for (uint32_t i = 0; i < offScreenResourceManager_.GetOffscreenCount(); i++) {
		// RTVヒープ割り当て
		DescriptorHandles rtvHandles = 
			directXCommon_->AssignRtvHeap(offScreenResourceManager_.GetOffscreenResource(i), &directXCommon_->GetSwapChainRtvDesc());
		offScreenResourceManager_.SetRtvHandle(rtvHandles.cpuHandle_, i);
		// SRVヒープ割り当て
		DescriptorHandles srvHandles =
			directXCommon_->AssignSrvHeap(offScreenResourceManager_.GetOffscreenResource(i), offScreenResourceManager_.GetOffscreenSrvDesc());
		offScreenResourceManager_.SetSrvHandle(srvHandles, i);
	}
	// * ポストプロセスマネージャー初期化 * //
	rendaringPostprocess_ = RendaringPostprosecess::GetInstance();
	rendaringPostprocess_->Initialize(directXCommon_->GetDevice(), directXCommon_->GetCommandManager(D3D12_COMMAND_LIST_TYPE_DIRECT));
	rendaringPostprocess_->SetNormalPSO(graphicPipelineManager_->GetNormalPso());
	rendaringPostprocess_->SetColorCorrectionPSO(graphicPipelineManager_->GetColorCorrectionPso());
	rendaringPostprocess_->SetGrayScalePSO(graphicPipelineManager_->GetGrayScalePso());
	rendaringPostprocess_->SetVignettePSO(graphicPipelineManager_->GetVignettePso());
	rendaringPostprocess_->SetOffscreenResource(
		offScreenResourceManager_.GetOffscreenResource(0), offScreenResourceManager_.GetOffscreenResource(1));
	rendaringPostprocess_->SetOffscreenRtvHandle(
		offScreenResourceManager_.GetOffscreenRtvHandles(0), offScreenResourceManager_.GetOffscreenRtvHandles(1));
	rendaringPostprocess_->SetOffscreenSrvHandle(
		offScreenResourceManager_.GetOffscreenSrvHandles(0), offScreenResourceManager_.GetOffscreenSrvHandles(1));
	rendaringPostprocess_->SetDsvHandle(directXCommon_->GetDepthStencilViewHandle()->cpuHandle_);

	assetManager_ = AssetManager::GetInstance();
	assetManager_->Initalize(directXCommon_);

	editor_ = std::make_unique<OnWindowsEditor>();
	editor_->Initialize();

	frameCounter_.Initialize();
	graphRenderer_ = GraphRenderer::GetInstance();
	graphRenderer_->Initialize();

	inputInterface_ = InputInterface::GetInstance();
	inputInterface_->Initialize(
		dynamic_cast<GameWindowManager*>(gameWindowManager.get())->GetWindow("QuickForgeEngine"), hInstance_);

	sceneManager_ = SceneManager::GetInstance();
	sceneManager_->Initalize();

	luaScriptResourceManager_ = LuaScriptResourceManager::GetInstance();
	luaScriptResourceManager_->Initialize();

	physicsManager_ = PhysicsManager::GetInstance();
	physicsManager_->Initialize();

	colliderManager_ = ColliderManager::GetInstance();
	colliderManager_->Initialize();
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
			frameCounter_.FrameStart();
			Update();
			Draw();
			frameCounter_.FrameEnd();
		}
	}
}

void WindowsEngineCore::Shutdown() {
	colliderManager_->Finalize();
	physicsManager_->Finalize();
	luaScriptResourceManager_->Finalize();
	sceneManager_->Finalize();
	graphRenderer_->Finalize();
	inputInterface_->Finalize();
	assetManager_->Finalize();

	imguiFrameController_.EndImGui();
	directXCommon_->Shutdown();
	gameWindowManager->Shutdown();
#ifdef _DEBUG
	DebugLog("FinalizeEngine");
#endif // _DEBUG
}

// この先はプライベート関数
void WindowsEngineCore::Update() {
	inputInterface_->Update();
	gameWindowManager->Update();
	editor_->Update();
	sceneManager_->Update();
}

void WindowsEngineCore::Draw() {
	assetManager_->PreDraw();
	directXCommon_->PreDraw();
	rendaringPostprocess_->SetBackBufferRtvHandle(directXCommon_->GetCurrentBackBufferCpuHandle());
	rendaringPostprocess_->PreDraw();
	imguiFrameController_.BeginFrame();
	graphRenderer_->PreDraw();
	sceneManager_->PreDraw();

	gameWindowManager->Draw();
	editor_->Draw();
	sceneManager_->Draw();

	sceneManager_->PostDraw();
	graphRenderer_->PostDraw();
	rendaringPostprocess_->PostDraw();
	imguiFrameController_.EndFrame(directXCommon_->GetCurrentBackBufferCpuHandle());
	directXCommon_->PostDraw();

	assetManager_->EndFrame();
	luaScriptResourceManager_->EndFrame();
	sceneManager_->EndFrame();
}
