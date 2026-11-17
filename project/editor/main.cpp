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
#include <stdexcept>
#include <nlohmann/json.hpp>

/// /// @brief Windowsアプリケーションのテスト
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {
	// 相対パスは起動時の作業ディレクトリを基準にする。
	QFE::EDITOR::EditorConfig projectConfig;
	if (!projectConfig.Load(L"EditorConfig.json")) {
		MessageBoxW(nullptr, L"Could not load EditorConfig.json. Select a project using editor/SelectProject.bat.", L"QuickForge", MB_OK | MB_ICONERROR);
		return -1;
	}
	// プロジェクトのルートを絶対パスに変換し、assets 以下のディレクトリを作成する。
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
	// 開始シーンがない場合は既存の DefaultScene を優先し、なければ作成する。
	const auto projectFile = projectRoot / L"project.json";
	const auto scenesDirectory = QFE::PROJECT::Assets() / L"scenes";
	std::filesystem::path startupScene;
	try {
		nlohmann::json project;
		// 既存の設定を読み込み、まだなければプロジェクトの初期設定を用意する。
		const bool projectExists = std::filesystem::exists(projectFile);
		if (projectExists) {
			std::ifstream input(projectFile, std::ios::binary);
			project = nlohmann::json::parse(input);
			if (!project.is_object()) throw std::runtime_error("Invalid project.json");
		} else {
			project = { {"name", QFE::ConvertString(projectRoot.filename().wstring())}, {"version", 1} };
		}
		const auto configured = project.find("start_scene");
		bool updateProject = false;
		if (configured != project.end() && !configured->is_string()) throw std::runtime_error("Invalid start_scene");
		const std::string sceneName = configured == project.end() ? std::string{} : configured->get<std::string>();
		if (!sceneName.empty()) {
			// start_scene は assets/scenes からの相対パスとして解決する。
			const std::filesystem::path relativeScene(QFE::ConvertString(sceneName));
			startupScene = scenesDirectory / relativeScene;
			if (relativeScene.is_absolute() || !QFE::PROJECT::IsWithin(startupScene, scenesDirectory)) {
				throw std::runtime_error("start_scene must be inside assets/scenes");
			}
		}
		// 指定がない、またはファイルがない場合はデフォルトシーンを使う。
		if (startupScene.empty() || !std::filesystem::exists(startupScene)) {
			startupScene = scenesDirectory / L"DefaultScene.scene";
			if (!QFE::PROJECT::IsWithin(startupScene, scenesDirectory)) throw std::runtime_error("Invalid DefaultScene path");
			if (!std::filesystem::exists(startupScene)) {
				// 初期カメラとライトを含む現在のシーンを保存する。
				std::ofstream scene(startupScene, std::ios::binary);
				scene << engineSystems.sceneManager->CreateSnapshot().sceneJson.dump(4) << '\n';
				scene.close();
				if (!scene) throw std::runtime_error("Could not create DefaultScene");
			}
			project["start_scene"] = "DefaultScene.scene";
			updateProject = true;
		}
		// プロジェクト設定を書き換える前に、開始シーンの基本形式を確認する。
		std::ifstream sceneFile(startupScene, std::ios::binary);
		const auto scene = nlohmann::json::parse(sceneFile);
		if (!scene.is_object() || !scene.contains("entities") || !scene["entities"].is_array()) {
			throw std::runtime_error("Invalid startup scene");
		}
		// デフォルトシーンを使う場合は、次回以降も同じシーンを開けるようにする。
		if (updateProject) {
			std::ofstream file(projectFile, std::ios::binary | std::ios::trunc);
			file << project.dump(4) << '\n';
			file.close();
			if (!file) throw std::runtime_error("Could not save project.json");
		}
	} catch (const std::exception& error) {
		QFE_LOG(std::string("Failed to initialize project: ") + error.what(), QFE::LogLevel::Error);
		MessageBoxW(nullptr, L"Could not initialize the project or its startup scene. Check project.json and the scene file.", L"QuickForge", MB_OK | MB_ICONERROR);
		QFE::FRAMEWORK::ShutdownWindowsQuickForgeEngineSystems(engineSystems);
		return -1;
	}
	// プロジェクトのアセット管理を開いてからモデルローダーを初期化する。
	std::string assetError;
	if (!engineSystems.assetManager->Open(projectRoot, assetError)) {
		QFE_LOG("Failed to open asset registry: " + assetError, QFE::LogLevel::Error);
		QFE::FRAMEWORK::ShutdownWindowsQuickForgeEngineSystems(engineSystems);
		return -1;
	}
	engineSystems.modelLoader->Initialize();
	// アセットと Lua スクリプトの参照先を選択されたプロジェクトに切り替える。
	engineResources.assetDir = QFE::ConvertString(QFE::PROJECT::Assets().wstring()) + "/";
	engineResources.modelDir = engineResources.assetDir;
	engineResources.config.scriptDir = QFE::ConvertString((QFE::PROJECT::Assets() / "scripts").wstring());
	engineSystems.luaScriptSystem->Reset();
	engineSystems.luaScriptSystem->Initialize(std::filesystem::path(QFE::ConvertString(engineResources.config.scriptDir)));
	// QuickForgeエンジンの初期化
	QFE::FRAMEWORK::EngineInitialize(engineSystems, engineResources);
	auto& graphicEngine = engineSystems.graphicEngine;
	QFE::SCENE::SceneManager& sceneManager = *engineSystems.sceneManager;
	try {
		sceneManager.LoadCurrentSceneFromJson(QFE::ConvertString(startupScene.wstring()));
	} catch (const std::exception& error) {
		QFE_LOG(std::string("Failed to load startup scene: ") + error.what(), QFE::LogLevel::Error);
		MessageBoxW(nullptr, L"Could not load the project's startup scene.", L"QuickForge", MB_OK | MB_ICONERROR);
		QFE::FRAMEWORK::ShutdownWindowsQuickForgeEngineSystems(engineSystems);
		return -1;
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
