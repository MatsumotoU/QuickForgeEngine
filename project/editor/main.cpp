#define NOMINMAX
#include <Windows.h>

#include "framework/application/WindowsEngineFramework.h"
#include "framework/application/WindowsGameModule.h"
#include "framework/application/WindowsEngineResources.h"
#include "framework/application/LuaScriptSystem.h"

#include "framework/graphic/GraphicEngineCoreFramework.h"
#include "framework/window/WindowCoreFrameWork.h"
#include "framework/window/WindowDialogFramework.h"
#include "framework/gui/GuiCoreFramework.h"
#include "framework/input/InputFrameWork.h"


#include "scene/SceneManager.h"
#include "game/shooting-editor/ShootingInspectorRegistration.h"
#include "core/logger/MyDebugLog.h"
#include "core/process/ProcessUtil.h"
#include "core/string/MyString.h"
#include "core/timer/FPSCounter.h"

#include "assetfactory/model/AssimpModelLoader.h"

#include "core/math/transform/Transform.h"

#include <imgui/imgui.h>

#include "GameEditor.h"
#include "ProjectSelector.h"
#include "assetfactory/game-data/GameProjectPaths.h"

#include <array>
#include <filesystem>
#include <vector>

namespace {
	bool IsRegularFile(const std::filesystem::path& path) {
		std::error_code error;
		return std::filesystem::is_regular_file(path, error) && !error;
	}

	std::filesystem::path GetExecutableDirectory() {
		std::array<wchar_t, 32768> executablePath{};
		const DWORD length = GetModuleFileNameW(
			nullptr, executablePath.data(), static_cast<DWORD>(executablePath.size()));
		if (length == 0 || length >= static_cast<DWORD>(executablePath.size())) {
			return {};
		}
		return std::filesystem::path(executablePath.data(), executablePath.data() + length)
			.parent_path();
	}

	std::filesystem::path FindCommandProcessor() {
		std::array<wchar_t, 32768> systemDirectory{};
		const UINT length = GetSystemDirectoryW(
			systemDirectory.data(), static_cast<UINT>(systemDirectory.size()));
		if (length == 0 || length >= static_cast<UINT>(systemDirectory.size())) {
			return {};
		}
		const std::filesystem::path commandProcessor =
			std::filesystem::path(systemDirectory.data(), systemDirectory.data() + length) /
			L"cmd.exe";
		return IsRegularFile(commandProcessor) ? commandProcessor : std::filesystem::path{};
	}

	std::filesystem::path FindRepositoryRootFrom(std::filesystem::path directory) {
		while (!directory.empty()) {
			if (IsRegularFile(directory / L"project" / L"tools" / L"BuildProject.bat")) {
				return directory;
			}
			const std::filesystem::path parent = directory.parent_path();
			if (parent == directory) {
				break;
			}
			directory = parent;
		}
		return {};
	}

	std::filesystem::path FindRepositoryRoot() {
		std::error_code error;
		const std::filesystem::path workingDirectory = std::filesystem::current_path(error);
		if (!error) {
			const std::filesystem::path root = FindRepositoryRootFrom(workingDirectory);
			if (!root.empty()) {
				return root;
			}
		}
		return FindRepositoryRootFrom(GetExecutableDirectory());
	}

	std::wstring GetBuildConfiguration() {
#if defined(QFE_MODE_DEBUG)
		return L"Debug";
#elif defined(QFE_MODE_DEVELOPMENT)
		return L"Development";
#elif defined(QFE_MODE_RELEASE)
		return L"Release";
#else
#error QuickForgeEngine build configuration is not defined.
#endif
	}

	bool BuildGameModule(const std::filesystem::path& projectPath) {
		const std::filesystem::path repositoryRoot = FindRepositoryRoot();
		if (repositoryRoot.empty()) {
			QFE_LOG("Failed to find the QuickForgeEngine repository root.", QFE::LogLevel::Error);
			return false;
		}

		const std::filesystem::path buildScript =
			repositoryRoot / L"project" / L"tools" / L"BuildProject.bat";
		const std::filesystem::path commandProcessor = FindCommandProcessor();
		if (!IsRegularFile(buildScript) || commandProcessor.empty()) {
			QFE_LOG("Failed to find BuildProject.bat or cmd.exe.", QFE::LogLevel::Error);
			return false;
		}

		const std::filesystem::path buildLogDirectory = repositoryRoot / L"generated" / L"logs";
		std::error_code error;
		std::filesystem::create_directories(buildLogDirectory, error);
		if (error) {
			QFE_LOG("Failed to create the external build log directory.", QFE::LogLevel::Error);
			return false;
		}
		const std::filesystem::path buildLogPath =
			buildLogDirectory / (L"GameModuleBuild-" + std::to_wstring(GetCurrentProcessId()) + L".log");

		const std::wstring commandArguments =
			L"/d /s /c \"\"" + buildScript.wstring() + L"\" \"" +
			projectPath.wstring() + L"\" /p:Configuration=" +
			GetBuildConfiguration() + L" /p:Platform=x64 /p:BuildProjectReferences=false /fl /flp:\"logfile=" +
			buildLogPath.wstring() + L"\"\"";

		QFE_LOG("Building selected project: " + QFE::ConvertString(projectPath.wstring()), QFE::LogLevel::EditorInfo);
		uint32_t exitCode = UINT32_MAX;
		if (!QFE::ProcessUtil::RunExeAndWait(
			commandProcessor.wstring(),
			commandArguments,
			repositoryRoot.wstring(),
			exitCode)) {
			QFE_LOG(
				"Failed to start or wait for BuildProject.bat.",
				QFE::LogLevel::Error);
			return false;
		}
		if (exitCode != 0) {
			QFE_LOG(
				"Project build failed with exit code " + std::to_string(exitCode) +
				". Log: " + QFE::ConvertString(buildLogPath.wstring()),
				QFE::LogLevel::Error);
			return false;
		}

		QFE_LOG("Project build succeeded.", QFE::LogLevel::EditorInfo);
		return true;
	}
}

/// /// @brief Windowsアプリケーションのテスト
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {
	// QuickForgeエンジンのリソースを初期化
	const auto repositoryRoot = FindRepositoryRoot();
	if (repositoryRoot.empty()) { MessageBoxW(nullptr, L"Could not locate the QuickForgeEngine repository.", L"QuickForge", MB_OK | MB_ICONERROR); return -1; }
	std::error_code workingDirectoryError;
	std::filesystem::current_path(repositoryRoot / L"project", workingDirectoryError);
	if (workingDirectoryError) { MessageBoxW(nullptr, L"Could not open the engine working directory.", L"QuickForge", MB_OK | MB_ICONERROR); return -1; }
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
	const std::filesystem::path editorConfigPath = repositoryRoot / L"project" / L"editor" / L"editor-config.json";
	QFE::EDITOR::EditorConfig projectConfig;
	if (!projectConfig.Load(editorConfigPath)) {
		QFE_LOG("Failed to load editor-config.json; using the default game projects directory.", QFE::LogLevel::Warning);
	}
	QFE::EDITOR::ProjectSelector selector(projectConfig, editorConfigPath);
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
	engineResources.assetDir = QFE::ConvertString(QFE::PROJECT::Assets().wstring()) + "/";
	engineResources.modelDir = engineResources.assetDir;
	engineResources.config.scriptDir = QFE::ConvertString((QFE::PROJECT::Assets() / "scripts").wstring());
	engineSystems.luaScriptSystem->Reset();
	engineSystems.luaScriptSystem->Initialize(std::filesystem::path(QFE::ConvertString(engineResources.config.scriptDir)));
	// QuickForgeエンジンの初期化
	QFE::FRAMEWORK::EngineInitialize(engineSystems, engineResources);
	auto& graphicEngine = engineSystems.graphicEngine;
	QFE::SCENE::SceneManager& sceneManager = *engineSystems.sceneManager;

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
	gameEditor.Initialize(&sceneManager, sceneTextureId, *engineSystems.windowManager, *engineSystems.assetManager, editorConfig);

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
		QFE::FRAMEWORK::EndWindowsEngineFrame(engineSystems);

		// Fileメニューからの要求を、描画処理が完了したフレーム境界で実行する。
		if (gameEditor.ConsumeGameModuleBuildAndReloadRequest()) {
			std::wstring selectedProject;
			if (QFE::FRAMEWORK::RequestGetFilePathFromUser(
				*engineSystems.windowManager, L"Visual C++ project", L"*.vcxproj", selectedProject)) {
				const std::filesystem::path projectPath(selectedProject);
				if (IsRegularFile(projectPath) &&
					_wcsicmp(projectPath.extension().c_str(), L".vcxproj") == 0 &&
					BuildGameModule(projectPath)) {
					if (QFE::FRAMEWORK::ReloadWindowsGameModule(gameModule, engineSystems)) {
						QFE_LOG("ShootingGame.dll reload succeeded.", QFE::LogLevel::EditorInfo);
					} else {
						QFE_LOG("ShootingGame.dll reload failed.", QFE::LogLevel::Error);
					}
				}
			}
		}
	}

	QFE::FRAMEWORK::ShutdownWindowsQuickForgeEngineSystems(engineSystems);
	return 0;
}
