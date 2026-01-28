/**
 * @file WindowsEngineCore.cpp
 * @brief Windows向けエンジンコア実装
 */

#include "engine/include/WindowsEngineCore.h"
#include "editor/include/OnWindowsEditor.h"

#include <thread>
#include "engine/include/utility/DebugTool/ImGui/ImGuiInitializer.h"
#ifdef QFE_OPTIMIZE_OFF
#include "engine/include/utility/DebugTool/DebugLog/MyDebugLog.h"
#include "engine/include/renderer/ModelRenderer.h"
#endif // QFE_OPTIMIZE_OFF
#include "engine/include/core/EngineGlobalValue.h"
#include "engine/include/utility/FileSystems/FileUtility.h"

using namespace QFE;

namespace {
	uint32_t windowWidth = 1280;
	uint32_t windowHeight = 720;
}

/**
 * @brief コンストラクタ
 * @param hInstance
 * @param lpCmdLine
 */
WindowsEngineCore::WindowsEngineCore(HINSTANCE& hInstance, LPSTR& lpCmdLine)
	:debugCore_(lpCmdLine), hInstance_(hInstance), lpCmdLine_(lpCmdLine) {
}

void WindowsEngineCore::Initialize() {
	EngineGlobalValue::windowWidth = windowWidth;
	EngineGlobalValue::windowHeight = windowHeight;
	std::string windowTitle = "LE2A_14_マツモト_ユウタ";

#ifdef QFE_OPTIMIZE_OFF
	MyDebugLog::GetInstance()->Initialize();
#endif // QFE_OPTIMIZE_OFF
	// Create Window
	gameWindowManager = std::make_unique<GameWindowManager>();
	gameWindowManager->Initialize();
	gameWindowManager->AddWindow(windowWidth, windowHeight, windowTitle);
	// Initialize DirectXCommon
	directXCommon_ = DirectXCommon::GetInstance();
	directXCommon_->Initialize(
		dynamic_cast<GameWindowManager*>(gameWindowManager.get())->GetWindow(windowTitle), windowWidth, windowHeight);
	// Initialize ImGui
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
	// Initialize Other Managers
	graphicPipelineManager_ = GraphicPipelineManager::GetInstance();
	graphicPipelineManager_->Initialize(directXCommon_->GetDevice());


	offScreenResourceManager_.Initialize(directXCommon_->GetDevice(), windowWidth, windowHeight);

	for (uint32_t i = 0; i < offScreenResourceManager_.GetOffscreenCount(); i++) {

		DescriptorHandles rtvHandles =
			directXCommon_->AssignRtvHeap(offScreenResourceManager_.GetOffscreenResource(i), &directXCommon_->GetSwapChainRtvDesc());
		offScreenResourceManager_.SetRtvHandle(rtvHandles.cpuHandle_, i);

		DescriptorHandles srvHandles =
			directXCommon_->AssignSrvHeap(offScreenResourceManager_.GetOffscreenResource(i), offScreenResourceManager_.GetOffscreenSrvDesc());
		offScreenResourceManager_.SetSrvHandle(srvHandles, i);
	}

	renderingPostprocess_ = RenderingPostprocess::GetInstance();
	renderingPostprocess_->Initialize(directXCommon_->GetDevice(), directXCommon_->GetCommandManager(D3D12_COMMAND_LIST_TYPE_DIRECT));
	renderingPostprocess_->SetNormalPSO(graphicPipelineManager_->GetNormalPso());
	renderingPostprocess_->SetColorCorrectionPSO(graphicPipelineManager_->GetColorCorrectionPso());
	renderingPostprocess_->SetGrayScalePSO(graphicPipelineManager_->GetGrayScalePso());
	renderingPostprocess_->SetVignettePSO(graphicPipelineManager_->GetVignettePso());
	renderingPostprocess_->SetPixelPSO(graphicPipelineManager_->GetPixelPso());
	renderingPostprocess_->SetOffscreenResource(
		offScreenResourceManager_.GetOffscreenResource(0), offScreenResourceManager_.GetOffscreenResource(1));
	renderingPostprocess_->SetOffscreenRtvHandle(
		offScreenResourceManager_.GetOffscreenRtvHandles(0), offScreenResourceManager_.GetOffscreenRtvHandles(1));
	renderingPostprocess_->SetOffscreenSrvHandle(
		offScreenResourceManager_.GetOffscreenSrvHandles(0), offScreenResourceManager_.GetOffscreenSrvHandles(1));
	renderingPostprocess_->SetDsvHandle(directXCommon_->GetDepthStencilViewHandle()->cpuHandle_);

	assetManager_ = AssetManager::GetInstance();
	assetManager_->Initialize(directXCommon_);

	editor_ = std::make_unique<OnWindowsEditor>();
	editor_->Initialize();

#ifdef QFE_OPTIMIZE_OFF
	DebugLog("======================Initialized OnWindowsEditor======================");
#endif // QFE_OPTIMIZE_OFF

	frameCounter_.Initialize();
	graphRenderer_ = Render::GraphRenderer::GetInstance();
	graphRenderer_->Initialize();

#ifdef QFE_OPTIMIZE_OFF
	DebugLog("======================InitializedGraphRenderer======================");
#endif // QFE_OPTIMIZE_OFF

	inputInterface_ = InputInterface::GetInstance();
	inputInterface_->Initialize(
		dynamic_cast<GameWindowManager*>(gameWindowManager.get())->GetWindow(windowTitle), hInstance_);

#ifdef QFE_OPTIMIZE_OFF
	DebugLog("======================Initialized InputInterface======================");
#endif // QFE_OPTIMIZE_OFF

	sceneManager_ = SceneManager::GetInstance();
	sceneManager_->Initialize();

#ifdef QFE_OPTIMIZE_OFF
	DebugLog("======================Initialized SceneManager======================");
#endif // QFE_OPTIMIZE_OFF

	luaScriptResourceManager_ = LuaScriptResourceManager::GetInstance();
	luaScriptResourceManager_->Initialize();

#ifdef QFE_OPTIMIZE_OFF
	DebugLog("======================Initialized LuaScriptResourceManager======================");
#endif // QFE_OPTIMIZE_OFF
	try {
		csScriptManager_ = CsharpVirtualEnvironmentOnQFE::GetInstance();
		csScriptManager_->Initialize();
	}
	catch (const std::exception& e) {
#ifdef QFE_OPTIMIZE_OFF
		DebugLog(std::string("Error: ") + e.what());
#endif // QFE_OPTIMIZE_OFF
	}

#ifdef QFE_OPTIMIZE_OFF
	DebugLog("======================Initialized CsharpVirtualEnvironmentOnQFE======================");
#endif // QFE_OPTIMIZE_OFF

	physicsManager_ = PhysicsManager::GetInstance();
	physicsManager_->Initialize();

#ifdef QFE_OPTIMIZE_OFF
	DebugLog("======================Initialized PhysicsManager======================");
#endif // QFE_OPTIMIZE_OFF

	colliderManager_ = ColliderManager::GetInstance();
	colliderManager_->Initialize();

#ifdef QFE_OPTIMIZE_OFF
	DebugLog("======================Initialized ColliderManager======================");
#endif // QFE_OPTIMIZE_OFF

	multiThreadTaskExecutor_ = MultiThreadTaskExecutor::GetInstance();
	multiThreadTaskExecutor_->Initialize();

#ifdef QFE_OPTIMIZE_OFF
	DebugLog("======================Initialized MultiThreadTaskExecutor======================");
#endif // QFE_OPTIMIZE_OFF

	audioInterface_ = AudioInterface::GetInstance();
	audioInterface_->Initialize();

#ifdef QFE_OPTIMIZE_OFF
	DebugLog("======================Initialized Engine======================");
#endif // QFE_OPTIMIZE_OFF
}


void WindowsEngineCore::MainLoop() {
	while (gameWindowManager->IsWindowActive())
	{

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
#ifdef QFE_OPTIMIZE_OFF
	DebugLog("FinalizeEngine");
	MyDebugLog::GetInstance()->Finalize();
#endif // QFE_OPTIMIZE_OFF
}


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
