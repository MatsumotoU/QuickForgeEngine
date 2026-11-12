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

#include <imgui/imgui.h>

#include "GameEditor.h"
#include "EditorConfig.h"
#include "assetfactory/game-data/GameProjectPaths.h"

#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>

/// /// @brief Windowsアプリケーションのテスト
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {
	// 相対パスは起動時の作業ディレクトリを基準にする。
	QFE::EDITOR::EditorConfig projectConfig;
	if (!projectConfig.Load(L"EditorConfig.json")) {
		MessageBoxW(nullptr, L"Could not load EditorConfig.json. Select a project using editor/SelectProject.bat.", L"QuickForge", MB_OK | MB_ICONERROR);
		return -1;
	}
	std::error_code pathError;
	const auto projectRoot = std::filesystem::absolute(std::filesystem::path(QFE::ConvertString(projectConfig.projectRoot)), pathError).lexically_normal();
	if (!pathError) {
		for (const auto* dir : { L"scenes", L"entities", L"models", L"images", L"audio", L"scripts", L"shaders", L"animations" }) {
			std::filesystem::create_directories(projectRoot / L"assets" / dir, pathError);
			if (pathError) break;
		}
	}
	if (pathError) {
		MessageBoxW(nullptr, L"Could not create project directories. Check EditorConfig.json.", L"QuickForge", MB_OK | MB_ICONERROR);
		return -1;
	}
	QFE::PROJECT::SetRoot(projectRoot);
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
	// 既存ファイルは上書きせず、必要な初期ファイルを作る。
	const auto projectFile = projectRoot / L"project.json";
	bool createdProject = false;
	if (!std::filesystem::exists(projectFile, pathError) && !pathError) {
		const auto scenePath = QFE::PROJECT::Assets() / L"scenes" / L"default.scene";
		if (!std::filesystem::exists(scenePath, pathError) && !pathError) {
			std::ofstream scene(scenePath, std::ios::binary);
			scene << engineSystems.sceneManager->CreateSnapshot().sceneJson.dump(4) << '\n';
			scene.close();
			if (!scene) pathError = std::make_error_code(std::errc::io_error);
		}
		if (!pathError) {
			std::ofstream file(projectFile, std::ios::binary);
			file << nlohmann::json({ {"name", QFE::ConvertString(projectRoot.filename().wstring())}, {"version", 1}, {"start_scene", "default.scene"} }).dump(4) << '\n';
			file.close();
			if (!file) pathError = std::make_error_code(std::errc::io_error);
			else createdProject = true;
		}
	}
	if (pathError) {
		MessageBoxW(nullptr, L"Could not initialize the project files.", L"QuickForge", MB_OK | MB_ICONERROR);
		QFE::FRAMEWORK::ShutdownWindowsQuickForgeEngineSystems(engineSystems);
		return -1;
	}
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
	if (createdProject) {
		sceneManager.LoadCurrentSceneFromJson(QFE::ConvertString((QFE::PROJECT::Assets() / L"scenes" / L"default.scene").wstring()));
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
