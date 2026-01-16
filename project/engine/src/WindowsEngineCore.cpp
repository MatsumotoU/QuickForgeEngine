/**
 * @file WindowsEngineCore.cpp
 * @brief Windows版エンジンのコアシステム実装
 */

#include "engine/include/WindowsEngineCore.h"
#include "editor/include/OnWindowsEditor.h"

#include <thread>
#include "engine/include/utility/DebugTool/ImGui/ImGuiInitializer.h"
#ifdef _DEBUG
#include "engine/include/utility/DebugTool/DebugLog/MyDebugLog.h"
#include "engine/include/renderer/ModelRenderer.h"
#endif // _DEBUG
#include "engine/include/core/EngineGlobalValue.h"
#include "engine/include/utility/FileSystems/FileUtility.h"

namespace {
	uint32_t windowWidth = 1280;
	uint32_t windowHeight = 720;
}

/**
 * @brief コンストラクタ
 * @param hInstance インスタンスハンドル
 * @param lpCmdLine コマンドライン引数
 */
WindowsEngineCore::WindowsEngineCore(HINSTANCE& hInstance, LPSTR& lpCmdLine) 
	:debugCore_(lpCmdLine),hInstance_(hInstance),lpCmdLine_(lpCmdLine){
}

/** @brief システム全体の初期化 */
void WindowsEngineCore::Initialize() {
	QFE::EngineGlobalValue::windowWidth = windowWidth;
	QFE::EngineGlobalValue::windowHeight = windowHeight;
	std::string windowTitle = "LE2A_14_マツモト_ユウタ";

	// * デバッグログ初期化 * //
	MyDebugLog::GetInstance()->Initialize();

	// * ウィンドウマネージャー初期化 * //
	gameWindowManager = std::make_unique<GameWindowManager>();
	gameWindowManager->Initialize();
	gameWindowManager->AddWindow(windowWidth, windowHeight, windowTitle);
	// * DirectX初期化 * //
	directXCommon_ = DirectXCommon::GetInstance();
	// TODO: GetWindowの結果が nullptr の場合の安全性が欠けている
	directXCommon_->Initialize(
		dynamic_cast<GameWindowManager*>(gameWindowManager.get())->GetWindow(windowTitle), windowWidth, windowHeight);
	// * ImGuiの初期化 * //
	imguiFrameController_.Initialize(
		dynamic_cast<GameWindowManager*>(gameWindowManager.get())->GetWindow(windowTitle),
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
	renderingPostprocess_ = RenderingPostprocess::GetInstance();
	renderingPostprocess_->Initialize(directXCommon_->GetDevice(), directXCommon_->GetCommandManager(D3D12_COMMAND_LIST_TYPE_DIRECT));
	renderingPostprocess_->SetNormalPSO(graphicPipelineManager_->GetNormalPso());
	renderingPostprocess_->SetColorCorrectionPSO(graphicPipelineManager_->GetColorCorrectionPso());
	renderingPostprocess_->SetGrayScalePSO(graphicPipelineManager_->GetGrayScalePso());
	renderingPostprocess_->SetVignettePSO(graphicPipelineManager_->GetVignettePso());
	renderingPostprocess_->SetPixelPSO(graphicPipelineManager_->GetPixcelPso());
	renderingPostprocess_->SetOffscreenResource(
		offScreenResourceManager_.GetOffscreenResource(0), offScreenResourceManager_.GetOffscreenResource(1));
	renderingPostprocess_->SetOffscreenRtvHandle(
		offScreenResourceManager_.GetOffscreenRtvHandles(0), offScreenResourceManager_.GetOffscreenRtvHandles(1));
	renderingPostprocess_->SetOffscreenSrvHandle(
		offScreenResourceManager_.GetOffscreenSrvHandles(0), offScreenResourceManager_.GetOffscreenSrvHandles(1));
	renderingPostprocess_->SetDsvHandle(directXCommon_->GetDepthStencilViewHandle()->cpuHandle_);

	assetManager_ = AssetManager::GetInstance();
	assetManager_->Initalize(directXCommon_);

	editor_ = std::make_unique<OnWindowsEditor>();
	editor_->Initialize();

#ifdef _DEBUG
	DebugLog("======================Initialized OnWindowsEditor======================");
#endif // _DEBUG

	frameCounter_.Initialize();
	graphRenderer_ = GraphRenderer::GetInstance();
	graphRenderer_->Initialize();

#ifdef _DEBUG
	DebugLog("======================InitializedGraphRenderer======================");
#endif // _DEBUG

	inputInterface_ = InputInterface::GetInstance();
	inputInterface_->Initialize(
		dynamic_cast<GameWindowManager*>(gameWindowManager.get())->GetWindow(windowTitle), hInstance_);

#ifdef _DEBUG
	DebugLog("======================Initialized InputInterface======================");
#endif // _DEBUG

	sceneManager_ = SceneManager::GetInstance();
	sceneManager_->Initalize();

#ifdef _DEBUG
	DebugLog("======================Initialized SceneManager======================");
#endif // _DEBUG

	luaScriptResourceManager_ = LuaScriptResourceManager::GetInstance();
	luaScriptResourceManager_->Initialize();

#ifdef _DEBUG
	DebugLog("======================Initialized LuaScriptResourceManager======================");
#endif // _DEBUG
	try {
		csScriptManager_ = CsharpVirtualEnvironmentOnQFE::GetInstance();
		csScriptManager_->Initialize();
	}
	catch (const std::exception& e) {
#ifdef _DEBUG
		DebugLog(std::string("Error: ") + e.what());
#endif // _DEBUG
	}

#ifdef _DEBUG
	DebugLog("======================Initialized CsharpVirtualEnvironmentOnQFE======================");
#endif // _DEBUG

	physicsManager_ = PhysicsManager::GetInstance();
	physicsManager_->Initialize();

#ifdef _DEBUG
	DebugLog("======================Initialized PhysicsManager======================");
#endif // _DEBUG

	colliderManager_ = ColliderManager::GetInstance();
	colliderManager_->Initialize();

#ifdef _DEBUG
	DebugLog("======================Initialized ColliderManager======================");
#endif // _DEBUG

	multiThreadTaskExecutor_ = MultiThreadTaskExecutor::GetInstance();
	multiThreadTaskExecutor_->Initialize();

#ifdef _DEBUG
	DebugLog("======================Initialized MultiThreadTaskExecutor======================");
#endif // _DEBUG

	audioInterface_ = AudioInterface::GetInstance();
	audioInterface_->Initialize();

#ifdef _DEBUG
	DebugLog("======================Initialized Engine======================");
#endif // _DEBUG
}

/** @brief メインループ実行 */
void WindowsEngineCore::MainLoop() {
	while (gameWindowManager->IsWindowActive())
	{
		
		// アプリケーション安全終了用
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

/** @brief 終了処理 */
void WindowsEngineCore::Shutdown() {
	audioInterface_->Finalize();
	multiThreadTaskExecutor_->Finalize();
	colliderManager_->Finalize();
	physicsManager_->Finalize();
	csScriptManager_->Finalize();
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
	MyDebugLog::GetInstance()->Finalize();
}

// こ�E先�Eプライベ�Eト関数
void WindowsEngineCore::Update() {
	inputInterface_->Update();
	gameWindowManager->Update();
	editor_->Update();
	sceneManager_->Update();

	inputInterface_->EndFrame();
}

void WindowsEngineCore::Draw() {
	assetManager_->PreDraw();
	directXCommon_->PreDraw();
	renderingPostprocess_->SetBackBufferRtvHandle(directXCommon_->GetCurrentBackBufferCpuHandle());
	renderingPostprocess_->PreDraw();
	imguiFrameController_.BeginFrame();
	graphRenderer_->PreDraw();
	sceneManager_->PreDraw();

	gameWindowManager->Draw();
	editor_->Draw();
	sceneManager_->Draw();

	sceneManager_->PostDraw();
	graphRenderer_->PostDraw();
	renderingPostprocess_->PostDraw();
	imguiFrameController_.EndFrame(directXCommon_->GetCurrentBackBufferCpuHandle());
	directXCommon_->PostDraw();

	assetManager_->EndFrame();
	luaScriptResourceManager_->EndFrame();
	sceneManager_->EndFrame();
	multiThreadTaskExecutor_->FrameEnd();
}
