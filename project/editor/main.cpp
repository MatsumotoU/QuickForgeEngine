#define NOMINMAX
#include <Windows.h>

#include "framework/application/WindowsEngineFramework.h"
#include "framework/application/WindowsGameModule.h"
#include "framework/application/WindowsEngineResources.h"
#include "framework/application/LuaScriptSystem.h"

#include "framework/graphic/GraphicEngineCoreFramework.h"
#include "framework/window/WindowCoreFrameWork.h"
#include "framework/gui/GuiCoreFramework.h"
#include "framework/input/InputFrameWork.h"


#include "scene/SceneManager.h"
#include "game/shooting-editor/ShootingInspectorRegistration.h"
#include "core/logger/MyDebugLog.h"
#include "core/string/MyString.h"
#include "core/timer/FPSCounter.h"

#include "assetfactory/model/AssimpModelLoader.h"

#include "core/math/transform/Transform.h"

#include <imgui/imgui.h>

#include "GameEditor.h"
#include "ProjectSelector.h"
#include "assetfactory/game-data/GameProjectPaths.h"

#include <filesystem>

/// /// @brief Windowsアプリケーションのテスト
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {
	// 作業ディレクトリは project/、エディタ設定は editor/ 直下を使う。
	const std::filesystem::path editorConfigPath = L"editor/editor-config.json";
	QFE::EDITOR::EditorConfig projectConfig;
	if (!projectConfig.Load(editorConfigPath)) {
		MessageBoxW(nullptr, L"Could not load editor/editor-config.json. Start the editor from the project directory and check the configuration file.", L"QuickForge", MB_OK | MB_ICONERROR);
		return -1;
	}
	// QuickForgeエンジンのリソースを初期化
	QFE::FRAMEWORK::WindowsEngineResources engineResources;
	const bool engineConfigLoaded = engineResources.config.Load();
	const auto editorConfig = engineResources.config;
	engineResources.ApplyConfig(true);
	QFE::FRAMEWORK::WindowsGameModule gameModule;
	if (!gameModule.Load(L"ShootingGame.dll")) {
		QFE_LOG("Failed to load ShootingGame.dll");
		return -1;
	}
	// QuickForgeエンジンのシステムを生成
	QFE::FRAMEWORK::WindowsQuickForgeEngineSystems engineSystems;
	if (!QFE::FRAMEWORK::CreateWindowsQuickForgeEngineSystems(
		hInstance,
		engineResources.windowName,
		engineResources.windowWidth, engineResources.windowHeight,
		engineSystems,
		gameModule.GetComponentRegistrationFunction(), engineResources.config.scriptDir)) {
		return -1;
	}
	if (!engineConfigLoaded) QFE::DebugLog("Failed to load EngineConfig.json; using defaults.", QFE::LogLevel::Warning);
	QFE::EDITOR::ProjectSelector selector(projectConfig, editorConfigPath, *engineSystems.sceneManager);
	while (selector.SelectedProject().empty() && !selector.Cancelled() &&
		QFE::FRAMEWORK::CanLoopWindowManager(*engineSystems.windowManager)) {
		if (!QFE::FRAMEWORK::PreDrawGraphicEngine(engineSystems.graphicEngine.get())) break;
		QFE::FRAMEWORK::SetSwapChainRenderTarget(*engineSystems.graphicEngine);
		QFE::FRAMEWORK::PreDrawGui(*engineSystems.guiManager);
		selector.Draw(*engineSystems.windowManager);
		QFE::FRAMEWORK::PostDrawGui(*engineSystems.guiManager);
		QFE::FRAMEWORK::PostDrawGraphicEngine(engineSystems.graphicEngine.get());
	}
	const auto projectRoot = selector.SelectedProject();
	if (projectRoot.empty()) {
		QFE::FRAMEWORK::ShutdownWindowsQuickForgeEngineSystems(engineSystems);
		return 0;
	}
	// 選択したパスはエディタの設定へ保存し、実行中はプロセス内の共有コンテキストに渡す。
	projectConfig.lastProject = QFE::ConvertString(projectRoot.wstring());
	if (!projectConfig.Save(editorConfigPath)) QFE_LOG("Failed to save the selected project to editor-config.json.", QFE::LogLevel::Warning);
	QFE::PROJECT::SetRoot(projectRoot);
	std::string assetError;
	if (!engineSystems.assetManager->Open(projectRoot, assetError)) {
		QFE_LOG("Failed to open asset registry: " + assetError, QFE::LogLevel::Error);
		QFE::FRAMEWORK::ShutdownWindowsQuickForgeEngineSystems(engineSystems);
		return -1;
	}
	engineSystems.modelLoader->Initialize();
	engineResources.assetDir = QFE::ConvertString(QFE::PROJECT::Assets().wstring()) + "/";
	engineResources.modelDir = engineResources.assetDir;
	engineResources.config.scriptDir = QFE::ConvertString((QFE::PROJECT::Assets() / "scripts").wstring());
	engineSystems.luaScriptSystem->Reset();
	engineSystems.luaScriptSystem->Initialize(std::filesystem::path(QFE::ConvertString(engineResources.config.scriptDir)));
	// QuickForgeエンジンの初期化
	QFE::FRAMEWORK::EngineInitialize(engineSystems, engineResources);
	auto& graphicEngine = engineSystems.graphicEngine;
	QFE::SCENE::SceneManager& sceneManager = *engineSystems.sceneManager;
	if (selector.CreatedProject()) {
		sceneManager.LoadCurrentSceneFromJson(QFE::ConvertString((projectRoot / L"assets" / L"scenes" / L"default.scene").wstring()));
	}

	// エディタ用のシーンテクスチャ（レンダーターゲット）の作成
	QFE::GRAPHIC::RenderTargetHandle sceneRenderTargetHandle;
	QFE::GRAPHIC::DirectXResourceHandle sceneTextureHandle;
	uintptr_t sceneTextureGuiId = 0;
	if (!QFE::GRAPHIC::CreateEditorSceneTexture(
		graphicEngine.get(),
		sceneRenderTargetHandle,
		sceneTextureHandle,
		sceneTextureGuiId,
		engineResources.windowWidth,
		engineResources.windowHeight)) {
		QFE::FRAMEWORK::ShutdownWindowsQuickForgeEngineSystems(engineSystems);
		return -1;
	}

	// レイトレーシング結果をEditorのシーンテクスチャへ出力する。
	engineResources.finalRenderTargetHandle = sceneRenderTargetHandle;
	ImTextureID sceneTextureId = static_cast<ImTextureID>(sceneTextureGuiId);

	QFE::EDITOR::GameEditor gameEditor;
	if (!QFE::GAME::RegisterShootingGameInspectors(
		gameEditor.GetComponentInspectorRegistry())) {
		QFE::FRAMEWORK::ShutdownWindowsQuickForgeEngineSystems(engineSystems);
		return -1;
	}
	gameEditor.Initialize(&sceneManager, sceneTextureId, *engineSystems.windowManager, *engineSystems.assetManager, *engineSystems.modelLoader, editorConfig);

	// メインループ
	while (QFE::FRAMEWORK::CanLoopWindowManager(*engineSystems.windowManager)) {
		if (!QFE::FRAMEWORK::BeginWindowsEngineFrame(engineSystems)) {
			break;
		}

		// エディタの更新処理
		gameEditor.Update(engineSystems, engineResources);

		// エンジンの描画前処理（フレーム開始時の共通処理）
		QFE::FRAMEWORK::EnginePreDraw(engineSystems, engineResources);

		// エディタの描画処理
		gameEditor.Draw();

		// エンジンの描画後処理（フレーム終了時の共通処理）
		QFE::FRAMEWORK::EnginePostDraw(engineSystems, engineResources);
		QFE::FRAMEWORK::EndWindowsEngineFrame(engineSystems, &engineResources);
	}

	QFE::FRAMEWORK::ShutdownWindowsQuickForgeEngineSystems(engineSystems);
	return 0;
}
