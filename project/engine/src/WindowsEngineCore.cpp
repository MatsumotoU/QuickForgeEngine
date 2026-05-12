/**
 * @file WindowsEngineCore.cpp
 * @brief Windows向けエンジンコア実装
 */

#include "engine/include/WindowsEngineCore.h"

#include <thread>
#include "engine/include/utility/DebugTool/ImGui/ImGuiInitializer.h"
#include "engine/include/core/EngineDefines.h"
#ifdef QFE_OPTIMIZE_OFF
#include "engine/include/renderer/ModelRenderer.h"
#endif // QFE_OPTIMIZE_OFF
#include "engine/include/core/Profiler.h"

#include "engine/include/core/EngineGlobalValue.h"
#include "engine/include/utility/FileSystems/FileUtility.h"

#include "engine/include/core/Bridge/WindowsBridgeCore.h"

#include "engine/include/core/Math/MyMath.h"

#include <new>

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

	audioInterface_ = nullptr;
	colliderManager_ = nullptr;
	physicsManager_ = nullptr;
	sceneManager_ = nullptr;
	graphRenderer_ = nullptr;
	inputInterface_ = nullptr;
	assetManager_ = nullptr;

	engineApp_ = nullptr;
}

void QFE::WindowsEngineCore::Initialize(std::unique_ptr<IEngineApp> app) {
	// グローバル変数の初期化
	EngineGlobalValue::windowWidth = windowWidth;
	EngineGlobalValue::windowHeight = windowHeight;
	std::string windowTitle = "QuickForgeEngine";

	// エンジンの設定ファイルを読み込む
	configFilePath_ = "engine/resources/EngineConfig.json";
	QFE::FILE::LoadFileToJson(configFilePath_, configJson_);

	// スレッド立ち上げ
	threadPool_ = std::make_unique<ThreadPool>();

#ifdef QFE_OPTIMIZE_OFF
	MyDebugLog::GetInstance()->Initialize();
	// キャッシュラインサイズのログを出力
	std::string logInitMessage = "Initialized MyDebugLog. Cache line size: " + std::to_string(std::hardware_destructive_interference_size) + " bytes.";
	QFE_LOG(logInitMessage);
#endif // QFE_OPTIMIZE_OFF

	// プロファイラーの初期化
	Profiler::GetInstance()->Initialize();

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

	// エンジンの設定ファイルに最後に開いたプロジェクトの名前が保存されていれば、そのプロジェクトのディレクトリを設定する
	if (configJson_.contains("lastProjectName")) {
		// プロジェクトのディレクトリの整合性を確認して、問題があれば修復する
		if (assetManager_->GetResourceDirectoryManager()->CheckDirectoryIntegrity() == false) {
			assetManager_->GetResourceDirectoryManager()->RepairDirectoryIntegrity();
#ifdef QFE_OPTIMIZE_OFF
			QFE_LOG("Repaired project directory integrity");
#endif // QFE_OPTIMIZE_OFF
		}
		assetManager_->GetResourceDirectoryManager()->SetProjectDirectory(configJson_["lastProjectName"]);
	} else {
#ifdef QFE_OPTIMIZE_OFF
		QFE_LOG("No last project name found, creating a new project directory");
#endif // QFE_OPTIMIZE_OFF
		// 最後に開いたプロジェクトの名前が保存されていない場合は、新しいプロジェクトのディレクトリを生成して設定する
		std::string defaultProjectName = "NewGameProject";
		assetManager_->GetResourceDirectoryManager()->SetProjectDirectory(defaultProjectName);
		configJson_["lastProjectName"] = defaultProjectName;
	}

	assetManager_->Initialize(directXCommon_);

#ifdef QFE_OPTIMIZE_OFF
	// エディタとエンジン間の橋渡し関数を登録
	bridgeProvider_ = QFE::BRIDGE::EngineBridgeProvider::GetInstance();
	bridgeProvider_->SetUpBridge(std::make_unique<QFE::WindowsBridgeCore>(this));
#endif // QFE_OPTIMIZE_OFF

	// エンジンアプリケーションの初期化
	if (!app) {
		QFE_REPORT_SYSTEM_ERROR("EngineApp instance is null during WindowsEngineCore initialization.", SystemError::Abort);
	}
	engineApp_ = std::move(app);
	engineApp_->Initialize();

#ifdef QFE_OPTIMIZE_OFF
	QFE_LOG("======================Initialized OnWindowsEditor======================");
#endif // QFE_OPTIMIZE_OFF

	frameCounter_.Initialize();
	graphRenderer_ = Render::GraphRenderer::GetInstance();
	graphRenderer_->Initialize();

#ifdef QFE_OPTIMIZE_OFF
	QFE_LOG("======================InitializedGraphRenderer======================");
#endif // QFE_OPTIMIZE_OFF

	inputInterface_ = InputInterface::GetInstance();
	inputInterface_->Initialize(
		dynamic_cast<GameWindowManager*>(gameWindowManager.get())->GetWindow(windowTitle), hInstance_);

#ifdef QFE_OPTIMIZE_OFF
	QFE_LOG("======================Initialized InputInterface======================");
#endif // QFE_OPTIMIZE_OFF

	sceneManager_ = SceneManager::GetInstance();
	sceneManager_->Initialize();

#ifdef QFE_OPTIMIZE_OFF
	QFE_LOG("======================Initialized SceneManager======================");
#endif // QFE_OPTIMIZE_OFF

	physicsManager_ = PhysicsManager::GetInstance();
	physicsManager_->Initialize();

#ifdef QFE_OPTIMIZE_OFF
	QFE_LOG("======================Initialized PhysicsManager======================");
#endif // QFE_OPTIMIZE_OFF

	colliderManager_ = ColliderManager::GetInstance();
	colliderManager_->Initialize();

#ifdef QFE_OPTIMIZE_OFF
	QFE_LOG("======================Initialized ColliderManager======================");
#endif // QFE_OPTIMIZE_OFF

	audioInterface_ = AudioInterface::GetInstance();
	audioInterface_->Initialize();

#ifdef QFE_OPTIMIZE_OFF
	QFE_LOG("======================Initialized Engine======================");
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
			Profiler::GetInstance()->FrameStart();
			frameCounter_.FrameStart();
			Update();
			Draw();
			frameCounter_.FrameEnd();
			Profiler::GetInstance()->FrameEnd();
		}
	}
}


void WindowsEngineCore::Shutdown() {
	// ブリッジの終了処理
	if (bridgeProvider_) {
		bridgeProvider_->FinalizeBridge();
	}
	
	// エンジンの設定ファイルに最後に開いたプロジェクトの名前を保存する
#ifdef QFE_OPTIMIZE_OFF
	QFE_LOG("Shutdown Engine");
	configJson_["lastProjectName"] = assetManager_->GetResourceDirectoryManager()->GetProjectName();
	QFE::FILE::SaveJSONToFile(configFilePath_, configJson_);
#endif // QFE_OPTIMIZE_OFF

	// 各マネージャーの終了処理
	if (audioInterface_) {
		audioInterface_->Finalize();
	}
	if (colliderManager_) {
		colliderManager_->Finalize();
	}
	if (physicsManager_) {
		physicsManager_->Finalize();
	}
	if (sceneManager_) {
		sceneManager_->Finalize();
	}
	if (graphRenderer_) {
		graphRenderer_->Finalize();
	}
	if (inputInterface_) {
		inputInterface_->Finalize();
	}
	if (assetManager_) {
		assetManager_->Finalize();
	}

	imguiFrameController_.EndImGui();
	directXCommon_->Shutdown();
	gameWindowManager->Shutdown();

	Profiler::GetInstance()->Finalize();

#ifdef QFE_OPTIMIZE_OFF
	QFE_LOG("FinalizeEngine");
	MyDebugLog::GetInstance()->Finalize();
#endif // QFE_OPTIMIZE_OFF
}


void WindowsEngineCore::Update() {
	inputInterface_->Update();
	gameWindowManager->Update();
	engineApp_->Update();
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
	engineApp_->Draw();
	sceneManager_->Draw();

	sceneManager_->PostDraw();
	graphRenderer_->PostDraw();
	renderingPostprocess_->PostDraw();
	imguiFrameController_.EndFrame(directXCommon_->GetCurrentBackBufferCpuHandle());
	directXCommon_->PostDraw();

	assetManager_->EndFrame();
	sceneManager_->EndFrame();
}
