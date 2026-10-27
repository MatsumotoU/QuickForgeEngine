#define NOMINMAX
#include <Windows.h>

#include "framework/application/WindowsEngineFramework.h"
#include "framework/application/WindowsGameModule.h"
#include "framework/application/WindowsEngineResources.h"

#include "framework/graphic/GraphicEngineCoreFramework.h"
#include "framework/window/WindowCoreFrameWork.h"
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

#include <array>
#include <filesystem>
#include <vector>

namespace {
	std::filesystem::path GetEnvironmentPath(const wchar_t* variableName) {
		const DWORD requiredSize = GetEnvironmentVariableW(variableName, nullptr, 0);
		if (requiredSize == 0) {
			return {};
		}

		std::vector<wchar_t> value(requiredSize);
		const DWORD length = GetEnvironmentVariableW(
			variableName, value.data(), static_cast<DWORD>(value.size()));
		if (length == 0 || length >= static_cast<DWORD>(value.size())) {
			return {};
		}
		return std::filesystem::path(value.data(), value.data() + length);
	}

	bool IsRegularFile(const std::filesystem::path& path) {
		std::error_code error;
		return std::filesystem::is_regular_file(path, error) && !error;
	}

	std::filesystem::path FindMsBuild() {
		const std::filesystem::path configuredPath = GetEnvironmentPath(L"MSBUILD_EXE_PATH");
		if (IsRegularFile(configuredPath)) {
			return configuredPath;
		}

		const std::filesystem::path visualStudioDirectory = GetEnvironmentPath(L"VSINSTALLDIR");
		const std::filesystem::path visualStudioMsBuild =
			visualStudioDirectory / L"MSBuild" / L"Current" / L"Bin" / L"MSBuild.exe";
		if (!visualStudioDirectory.empty() && IsRegularFile(visualStudioMsBuild)) {
			return visualStudioMsBuild;
		}

		const std::array<std::filesystem::path, 2> programDirectories = {{
			GetEnvironmentPath(L"ProgramFiles"),
			GetEnvironmentPath(L"ProgramFiles(x86)")
		}};
		constexpr std::array<const wchar_t*, 2> versions = {{ L"2022", L"2019" }};
		constexpr std::array<const wchar_t*, 4> editions = {{
			L"Enterprise", L"Professional", L"Community", L"BuildTools"
		}};
		for (const std::filesystem::path& programDirectory : programDirectories) {
			if (programDirectory.empty()) {
				continue;
			}
			for (const wchar_t* version : versions) {
				for (const wchar_t* edition : editions) {
					const std::filesystem::path candidate =
						programDirectory / L"Microsoft Visual Studio" / version / edition /
						L"MSBuild" / L"Current" / L"Bin" / L"MSBuild.exe";
					if (IsRegularFile(candidate)) {
						return candidate;
					}
				}
			}
		}

		std::array<wchar_t, 32768> searchResult{};
		const DWORD length = SearchPathW(
			nullptr,
			L"MSBuild.exe",
			nullptr,
			static_cast<DWORD>(searchResult.size()),
			searchResult.data(),
			nullptr);
		if (length == 0 || length >= static_cast<DWORD>(searchResult.size())) {
			return {};
		}
		return std::filesystem::path(searchResult.data(), searchResult.data() + length);
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
		const std::filesystem::path configuredPath = GetEnvironmentPath(L"ComSpec");
		if (IsRegularFile(configuredPath)) {
			return configuredPath;
		}

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
			if (IsRegularFile(
				directory / L"project" / L"game" / L"shooting" / L"ShootingGame.vcxproj")) {
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

	bool BuildShootingGameModule() {
		const std::filesystem::path repositoryRoot = FindRepositoryRoot();
		if (repositoryRoot.empty()) {
			QFE_LOG("Failed to find the QuickForgeEngine repository root.", QFE::LogLevel::Error);
			return false;
		}

		const std::filesystem::path msBuildPath = FindMsBuild();
		if (msBuildPath.empty()) {
			QFE_LOG("Failed to find MSBuild.exe.", QFE::LogLevel::Error);
			return false;
		}
		const std::filesystem::path visualStudioDirectory =
			msBuildPath.parent_path().parent_path().parent_path();
		const std::filesystem::path vcVarsPath =
			visualStudioDirectory / L"VC" / L"Auxiliary" / L"Build" / L"vcvars64.bat";
		const std::filesystem::path commandProcessor = FindCommandProcessor();
		if (!IsRegularFile(vcVarsPath) || commandProcessor.empty()) {
			QFE_LOG("Failed to find vcvars64.bat or cmd.exe.", QFE::LogLevel::Error);
			return false;
		}

		const std::filesystem::path projectPath =
			repositoryRoot / L"project" / L"game" / L"shooting" / L"ShootingGame.vcxproj";
		const std::filesystem::path buildLogDirectory = repositoryRoot / L"generated" / L"logs";
		std::error_code error;
		std::filesystem::create_directories(buildLogDirectory, error);
		if (error) {
			QFE_LOG("Failed to create the external build log directory.", QFE::LogLevel::Error);
			return false;
		}
		const std::filesystem::path buildLogPath =
			buildLogDirectory / L"ShootingGameBuild.log";

		const std::wstring configuration = GetBuildConfiguration();
		const std::wstring msBuildArguments =
			L"\"" + projectPath.wstring() + L"\"" +
			L" /t:Build /p:Configuration=" + configuration +
			L" /p:Platform=x64 /p:BuildProjectReferences=false" +
			L" /m:1 /nr:false /nologo /v:minimal";
		const std::wstring commandArguments =
			L"/d /c call \"" + vcVarsPath.wstring() + L"\" && \"" +
			msBuildPath.wstring() + L"\" " + msBuildArguments;

		QFE_LOG("Building ShootingGame.dll with external MSBuild...", QFE::LogLevel::EditorInfo);
		uint32_t exitCode = UINT32_MAX;
		if (!QFE::ProcessUtil::RunExeAndWait(
			commandProcessor.wstring(),
			commandArguments,
			repositoryRoot.wstring(),
			buildLogPath.wstring(),
			exitCode)) {
			QFE_LOG(
				"Failed to start or wait for external MSBuild. Log: " +
				QFE::ConvertString(buildLogPath.wstring()),
				QFE::LogLevel::Error);
			return false;
		}
		if (exitCode != 0) {
			QFE_LOG(
				"ShootingGame.dll build failed with exit code " + std::to_string(exitCode) +
				". Log: " + QFE::ConvertString(buildLogPath.wstring()),
				QFE::LogLevel::Error);
			return false;
		}

		QFE_LOG("ShootingGame.dll build succeeded.", QFE::LogLevel::EditorInfo);
		return true;
	}
}

/// /// @brief Windowsアプリケーションのテスト
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {
	// QuickForgeエンジンのリソースを初期化
	QFE::FRAMEWORK::WindowsEngineResources engineResources;
	engineResources.windowName = "ShootingGameEditor";
	engineResources.windowWidth = 1280;
	engineResources.windowHeight = 720;
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
		1280,
		720)) {
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
	gameEditor.Initialize(&sceneManager, sceneTextureId, *engineSystems.windowManager);

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
		if (gameEditor.ConsumeGameModuleBuildAndReloadRequest() &&
			BuildShootingGameModule()) {
			if (QFE::FRAMEWORK::ReloadWindowsGameModule(gameModule, engineSystems)) {
				QFE_LOG("ShootingGame.dll reload succeeded.", QFE::LogLevel::EditorInfo);
			} else {
				QFE_LOG("ShootingGame.dll reload failed.", QFE::LogLevel::Error);
			}
		}
	}

	QFE::FRAMEWORK::ShutdownWindowsQuickForgeEngineSystems(engineSystems);
	return 0;
}
