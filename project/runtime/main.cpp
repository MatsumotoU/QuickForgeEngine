#define NOMINMAX
#include <Windows.h>
#include <shellapi.h>

#include "framework/application/WindowsEngineFramework.h"
#include "framework/application/WindowsGameModule.h"
#include "framework/window/WindowCoreFrameWork.h"
#include "framework/application/WindowsEngineResources.h"


#include "scene/SceneManager.h"
#include "core/logger/MyDebugLog.h"
#include "core/file/FileUtility.h"
#include "assetfactory/game-data/GameProjectPaths.h"
#include <commdlg.h>
#include <fstream>
#include "core/string/MyString.h"
#include "core/timer/FPSCounter.h"
#include "input/interface/InputInterface.h"

#include "assetfactory/model/AssimpModelLoader.h"

#include "core/math/transform/Transform.h"

#include <imgui.h>

#include <array>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "EngineDefines.h"

#include "resources/shaders/shaderStructs/hlslTypeToCpp.h"

namespace {
	// 開発環境では実行ファイルが generated/ にあるため、エンジン共通シェーダーの基準を探す。
	// 配布時に実行ディレクトリに engine/ がある構成もそのまま使用できる。
	void SetEngineWorkingDirectory() {
		std::error_code error;
		if (std::filesystem::is_regular_file("engine/resources/EngineConfig.json", error)) return;
		std::array<wchar_t, 32768> executable{};
		const DWORD length = GetModuleFileNameW(nullptr, executable.data(), static_cast<DWORD>(executable.size()));
		if (!length || length >= executable.size()) return;
		auto directory = std::filesystem::path(executable.data()).parent_path();
		while (!directory.empty()) {
			for (const auto& candidate : { directory, directory / L"project" }) {
				error.clear();
				if (std::filesystem::is_regular_file(candidate / L"engine/resources/EngineConfig.json", error)) {
					std::filesystem::current_path(candidate, error);
					return;
				}
			}
			const auto parent = directory.parent_path();
			if (parent == directory) break;
			directory = parent;
		}
	}
	void AddSceneCandidates(
		std::vector<std::filesystem::path>& candidates,
		const std::filesystem::path& path) {
		candidates.push_back(path);
		if (path.extension().empty()) {
			std::filesystem::path scenePath = path;
			scenePath += ".scene";
			candidates.push_back(std::move(scenePath));
			std::filesystem::path jsonPath = path;
			jsonPath += ".json";
			candidates.push_back(std::move(jsonPath));
		}
	}

	struct Arguments { std::optional<std::string> scene; std::filesystem::path project; };
	Arguments GetArguments() {
		Arguments result; int count = 0;
		LPWSTR* args = CommandLineToArgvW(GetCommandLineW(), &count);
		if (!args) return result;
		for (int i = 1; i < count; ++i) {
			const std::wstring_view arg(args[i]);
			if (arg == L"--project" && i + 1 < count) result.project = args[++i];
			else if (arg == L"--scene" && i + 1 < count) result.scene = QFE::ConvertString(std::wstring(args[++i]));
			else if (!arg.starts_with(L"--") && !result.scene) result.scene = QFE::ConvertString(std::wstring(args[i]));
		}
		LocalFree(args); return result;
	}

	std::filesystem::path ChooseProject() {
		std::array<wchar_t, 32768> path{};
		OPENFILENAMEW dialog{};
		dialog.lStructSize = sizeof(dialog);
		dialog.lpstrFilter = L"Project settings (project.json)\0project.json\0\0";
		dialog.lpstrFile = path.data(); dialog.nMaxFile = static_cast<DWORD>(path.size());
		dialog.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
		return GetOpenFileNameW(&dialog) ? std::filesystem::path(path.data()).parent_path() : std::filesystem::path{};
	}
	std::optional<std::string> GetConfiguredStartScene(
		const std::filesystem::path& resourceDirectory) {
		(void)resourceDirectory;
		std::ifstream file(QFE::PROJECT::Root() / "project.json");
		try {
			const auto json = nlohmann::json::parse(file);
			const auto scene = json.value("start_scene", std::string{});
			if (!scene.empty()) return scene;
		} catch (const nlohmann::json::exception&) {}
		QFE_LOG("The project does not contain a valid start_scene.", QFE::LogLevel::Error);
		return std::nullopt;
	}

	std::optional<std::filesystem::path> ResolveStartupScene(
		const std::filesystem::path& resourceDirectory,
		const std::optional<std::string>& commandLineScene) {
		const std::optional<std::string> sceneName = commandLineScene.has_value()
			? commandLineScene
			: GetConfiguredStartScene(resourceDirectory);
		if (!sceneName.has_value()) return std::nullopt;

		const std::filesystem::path requestedPath(QFE::ConvertString(*sceneName));
		std::vector<std::filesystem::path> candidates;
		if (requestedPath.is_absolute() || QFE::PROJECT::Root().empty()) AddSceneCandidates(candidates, requestedPath);
		if (requestedPath.is_relative()) {
			// 旧形式の resources/scene/... も assets/scenes/... として扱う。
			if (!requestedPath.empty() && *requestedPath.begin() == "resources") {
				AddSceneCandidates(candidates, QFE::PROJECT::Resolve(requestedPath));
			}
			AddSceneCandidates(candidates, resourceDirectory / "scenes" / requestedPath);
			if (QFE::PROJECT::Root().empty()) AddSceneCandidates(candidates, resourceDirectory / "scene" / requestedPath);
			AddSceneCandidates(candidates, resourceDirectory / requestedPath);
		}

		for (const std::filesystem::path& candidate : candidates) {
			std::error_code error;
			if ((!commandLineScene || !requestedPath.is_absolute()) && !QFE::PROJECT::IsWithin(candidate, QFE::PROJECT::Assets() / "scenes")) continue;
			if (std::filesystem::is_regular_file(candidate, error)) {
				return candidate;
			}
		}

		QFE_LOG("Startup scene was not found: " + *sceneName, QFE::LogLevel::Error);
		return std::nullopt;
	}
}

/// /// @brief Windowsアプリケーションのテスト
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {

	const auto arguments = GetArguments();
	const auto project = arguments.project.empty() ? ChooseProject() : arguments.project;
	if (project.empty()) return 0;
	std::error_code projectPathError;
	const auto absoluteProject = std::filesystem::absolute(project, projectPathError);
	if (projectPathError || !std::filesystem::is_regular_file(absoluteProject / L"project.json")) {
		MessageBoxW(nullptr, L"Invalid project directory.", L"QuickForge Runtime", MB_OK | MB_ICONERROR); return -1;
	}
	SetEngineWorkingDirectory();
	QFE::PROJECT::SetRoot(absoluteProject);
	std::vector<int> objectNums;

	// QuickForgeエンジンのリソースを初期化
	QFE::FRAMEWORK::WindowsEngineResources engineResources;
	const bool engineConfigLoaded = engineResources.config.Load();
	engineResources.ApplyConfig(false);
	if (!QFE::PROJECT::Root().empty()) {
		engineResources.assetDir = QFE::ConvertString(QFE::PROJECT::Assets().wstring()) + "/";
		engineResources.modelDir = engineResources.assetDir;
		engineResources.config.scriptDir = QFE::ConvertString((QFE::PROJECT::Assets() / "scripts").wstring());
	}
	QFE::FRAMEWORK::WindowsGameModule gameModule;
	if (!gameModule.Load(L"ShootingGame.dll")) {
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
	std::string assetError;
	if (!engineSystems.assetManager->Open(QFE::PROJECT::Root(), assetError)) {
		QFE_LOG("Failed to open asset registry: " + assetError, QFE::LogLevel::Error);
		QFE::FRAMEWORK::ShutdownWindowsQuickForgeEngineSystems(engineSystems);
		return -1;
	}
	engineSystems.modelLoader->Initialize();
	if (!engineConfigLoaded) QFE::DebugLog("Failed to load EngineConfig.json; using defaults.", QFE::LogLevel::Warning);
	// QuickForgeエンジンの初期化
	QFE::FRAMEWORK::EngineInitialize(engineSystems, engineResources);
	const std::optional<std::filesystem::path> startupScene = ResolveStartupScene(
		engineResources.assetDir,
		arguments.scene);
	if (!startupScene.has_value()) {
		QFE::FRAMEWORK::ShutdownWindowsQuickForgeEngineSystems(engineSystems);
		return -1;
	}
	engineSystems.sceneManager->LoadCurrentSceneFromJson(QFE::ConvertString(startupScene->wstring()));

	auto& fpsCounter = engineSystems.fpsCounter;
	QFE::SCENE::SceneManager& sceneManager = *engineSystems.sceneManager;
	QFE::EntityManager& entityManager = sceneManager.GetCurrentSceneEntityManager();

	// メインループ
	while (QFE::FRAMEWORK::CanLoopWindowManager(*engineSystems.windowManager)) {
		if (!QFE::FRAMEWORK::BeginWindowsEngineFrame(engineSystems)) {
			break;
		}

		float deltaTime = fpsCounter->GetDeltaTime();

		if (!gameModule.Update(engineSystems, engineResources, deltaTime)) {
			break;
		}

		// カメラの更新
		engineResources.viewProj = QFE::FRAMEWORK::UpdateMainCamera(sceneManager, &engineResources.cameraPosition);

		// エンジンの描画前処理（フレーム開始時の共通処理）
		QFE::FRAMEWORK::EnginePreDraw(engineSystems, engineResources);

		ImGui::Begin("Debug Info");
		ImGui::Text("FPS: %.2f", fpsCounter->GetAverageFPS());
		ImGui::Text("Object Count: %zu", entityManager.GetActiveEntityIds().size());
		ImGui::End();

		// エンジンの描画後処理（フレーム終了時の共通処理）
		QFE::FRAMEWORK::EnginePostDraw(engineSystems, engineResources);
		QFE::FRAMEWORK::EndWindowsEngineFrame(engineSystems, &engineResources);

		// FPSが60fps付近のオブジェクト数をカウント
		if (fpsCounter->GetAverageFPS() > 59.0f && fpsCounter->GetAverageFPS() < 61.0f) {
			objectNums.push_back(static_cast<int>(entityManager.GetActiveEntityIds().size()));
		}
	}

	QFE::FRAMEWORK::ShutdownWindowsQuickForgeEngineSystems(engineSystems);
	return 0;
}
