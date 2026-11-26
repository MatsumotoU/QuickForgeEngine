#define NOMINMAX
#include <Windows.h>

#include "framework/application/WindowsEngineFramework.h"
#include "framework/application/WindowsGameModule.h"
#include "framework/application/WindowsEngineResources.h"
#include "framework/assets/WindowsAssetsFramework.h"

#include "framework/graphic/GraphicEngineCoreFramework.h"
#include "framework/window/WindowCoreFrameWork.h"
#include "framework/window/WindowDialogFramework.h"
#include "framework/gui/GuiCoreFramework.h"
#include "framework/input/InputFrameWork.h"
#include "framework/assets/AssetsCoreFramework.h"
#include "framework/scene/SceneCoreFramework.h"


#include "game/shooting-editor/ShootingInspectorRegistration.h"
#include "core/logger/MyDebugLog.h"
#include "core/string/MyString.h"

#include <imgui/imgui.h>

#include "GameEditor.h"
#include "EditorConfig.h"


#include <filesystem>
#include <stdexcept>

/// /// @brief Windowsアプリケーションのテスト
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {
	// 相対パスは起動時の作業ディレクトリを基準にする。
	QFE::EDITOR::EditorConfig projectConfig;
	if (!projectConfig.Load(L"EditorConfig.json")) {
		QFE::FRAMEWORK::ShowWindowError(L"Could not load EditorConfig.json. Select a project using editor/SelectProject.bat.", L"QuickForge");
		return -1;
	}
	// プロジェクトのルートを絶対パスに変換し、assets 以下のディレクトリを作成する。
	std::error_code pathError;
	const auto projectRoot = std::filesystem::absolute(std::filesystem::path(QFE::ConvertString(projectConfig.projectRoot)), pathError).lexically_normal();
	if (!pathError) {
		try {
			QFE::FRAMEWORK::CreateProjectAssets(projectRoot, L"assets");
		} catch (const std::filesystem::filesystem_error& error) {
			pathError = error.code();
		}
	}
	if (pathError) {
		QFE::FRAMEWORK::ShowWindowError(L"Could not create project directories. Check EditorConfig.json.", L"QuickForge");
		return -1;
	}
	QFE::FRAMEWORK::ASSETS::SetProjectRoot(projectRoot);

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
		gameModule.GetComponentRegistrationFunction())) {
		return -1;
	}
	if (!engineConfigLoaded) QFE::DebugLog("Failed to load EngineConfig.json; using defaults.", QFE::LogLevel::Warning);
	// プロジェクト設定と開始シーンを用意する。
	std::filesystem::path startupScene;
	try {
		startupScene = QFE::FRAMEWORK::InitializeProjectStartupScene(projectRoot, *engineSystems.sceneManager);
	} catch (const std::exception& error) {
		QFE_LOG(std::string("Failed to initialize project: ") + error.what(), QFE::LogLevel::Error);
		QFE::FRAMEWORK::ShowWindowError(L"Could not initialize the project or its startup scene. Check project.json and the scene file.", L"QuickForge");
		QFE::FRAMEWORK::ShutdownWindowsQuickForgeEngineSystems(engineSystems);
		return -1;
	}
	// プロジェクトのアセット管理を開いてからモデルローダーを初期化する。
	std::string assetError;
	if (!QFE::FRAMEWORK::InitializeProjectAssets(projectRoot, *engineSystems.assetManager,
		*engineSystems.modelLoader, engineResources.assetDir,
		engineResources.modelDir, assetError)) {
		QFE_LOG("Failed to open asset registry: " + assetError, QFE::LogLevel::Error);
		QFE::FRAMEWORK::ShutdownWindowsQuickForgeEngineSystems(engineSystems);
		return -1;
	}
	// QuickForgeエンジンの初期化
	QFE::FRAMEWORK::EngineInitialize(engineSystems, engineResources);
	auto& graphicEngine = engineSystems.graphicEngine;
	QFE::SCENE::SceneManager& sceneManager = *engineSystems.sceneManager;
	try {
		QFE::FRAMEWORK::LoadScene(sceneManager, QFE::ConvertString(startupScene.wstring()));
	} catch (const std::exception& error) {
		QFE_LOG(std::string("Failed to load startup scene: ") + error.what(), QFE::LogLevel::Error);
		QFE::FRAMEWORK::ShowWindowError(L"Could not load the project's startup scene.", L"QuickForge");
		QFE::FRAMEWORK::ShutdownWindowsQuickForgeEngineSystems(engineSystems);
		return -1;
	}

	// エディタ用のシーンテクスチャ（レンダーターゲット）の作成
	uintptr_t sceneTextureGuiId = 0;
	if (!QFE::FRAMEWORK::CreateEditorSceneTexture(
		graphicEngine.get(),
		engineResources.finalRenderTargetHandle,
		sceneTextureGuiId,
		engineResources.windowWidth,
		engineResources.windowHeight)) {
		QFE::FRAMEWORK::ShutdownWindowsQuickForgeEngineSystems(engineSystems);
		return -1;
	}

	// 作成した描画先にエンジンの描画結果を出力し、GUI から同じテクスチャを参照する。
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
		gameEditor.Draw(engineResources);

		// エンジンの描画後処理（フレーム終了時の共通処理）
		QFE::FRAMEWORK::EnginePostDraw(engineSystems, engineResources);
		QFE::FRAMEWORK::EndWindowsEngineFrame(engineSystems, &engineResources);
	}

	QFE::FRAMEWORK::ShutdownWindowsQuickForgeEngineSystems(engineSystems);
	return 0;
}
