#define NOMINMAX
#include <Windows.h>

#include "framework/application/WindowsEngineFramework.h"
#include "framework/application/WindowsGameModule.h"
#include "framework/window/WindowCoreFrameWork.h"
#include "framework/window/WindowDialogFramework.h"
#include "framework/application/WindowsEngineResources.h"
#include "framework/network/NetworkCoreFrame.h"


#include "framework/scene/SceneCoreFramework.h"
#include "core/logger/MyDebugLog.h"
#include "core/file/FileUtility.h"
#include "framework/assets/AssetsCoreFramework.h"
#include <fstream>
#include "core/string/MyString.h"
#include "core/timer/FPSCounter.h"

#include "core/math/transform/Transform.h"

#include <imgui.h>

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
		// 実行ファイルの OS 固有の取得方法は ApplicationFramework に任せる。
		auto directory = QFE::FRAMEWORK::GetApplicationExecutablePath().parent_path();
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

	struct Arguments {
		std::optional<std::string> scene;
		std::filesystem::path project;
		std::optional<bool> showColliders;
		std::string logHost;
		std::string logPort;
	};
	Arguments GetArguments() {
		Arguments result;
		const auto args = QFE::FRAMEWORK::GetApplicationArguments();
		for (size_t i = 1; i < args.size(); ++i) {
			const std::wstring_view arg(args[i]);
			if (arg == L"--project" && i + 1 < args.size()) result.project = args[++i];
			else if (arg == L"--scene" && i + 1 < args.size()) result.scene = QFE::ConvertString(args[++i]);
			else if (arg == L"--colliders=on") result.showColliders = true;
			else if (arg == L"--colliders=off") result.showColliders = false;
			else if (arg == L"--log-host" && i + 1 < args.size()) result.logHost = QFE::ConvertString(args[++i]);
			else if (arg == L"--log-port" && i + 1 < args.size()) result.logPort = QFE::ConvertString(args[++i]);
			else if (!arg.starts_with(L"--") && !result.scene) result.scene = QFE::ConvertString(args[i]);
		}
		return result;
	}

	std::filesystem::path ChooseProject() {
		// メインウィンドウ作成前に設定ファイルを選ぶ。
		std::wstring path;
		return QFE::FRAMEWORK::RequestStartupFilePath(L"Project settings (project.json)", L"project.json", path)
			? std::filesystem::path(path).parent_path() : std::filesystem::path{};
	}
	std::optional<std::string> GetConfiguredStartScene(
		const std::filesystem::path& resourceDirectory) {
		(void)resourceDirectory;
		std::ifstream file(QFE::FRAMEWORK::ASSETS::GetProjectRoot() / "project.json");
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
		if (requestedPath.is_absolute() || QFE::FRAMEWORK::ASSETS::GetProjectRoot().empty()) AddSceneCandidates(candidates, requestedPath);
		if (requestedPath.is_relative()) {
			// 旧形式の resources/scene/... も assets/scenes/... として扱う。
			if (!requestedPath.empty() && *requestedPath.begin() == "resources") {
				AddSceneCandidates(candidates, QFE::FRAMEWORK::ASSETS::ResolveProjectAsset(requestedPath));
			}
			AddSceneCandidates(candidates, resourceDirectory / "scenes" / requestedPath);
			if (QFE::FRAMEWORK::ASSETS::GetProjectRoot().empty()) AddSceneCandidates(candidates, resourceDirectory / "scene" / requestedPath);
			AddSceneCandidates(candidates, resourceDirectory / requestedPath);
		}

		for (const std::filesystem::path& candidate : candidates) {
			std::error_code error;
			if ((!commandLineScene || !requestedPath.is_absolute()) && !QFE::FRAMEWORK::ASSETS::IsPathWithin(candidate, QFE::FRAMEWORK::ASSETS::GetProjectAssets() / "scenes")) continue;
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
		QFE::FRAMEWORK::ShowWindowError(L"Invalid project directory.", L"QuickForge Runtime"); return -1;
	}
	SetEngineWorkingDirectory();
	QFE::FRAMEWORK::ASSETS::SetProjectRoot(absoluteProject);
	std::vector<int> objectNums;

	// QuickForgeエンジンのリソースを初期化
	QFE::FRAMEWORK::WindowsEngineResources engineResources;
	const bool engineConfigLoaded = engineResources.config.Load();
	engineResources.ApplyConfig(false);
	if (arguments.showColliders.has_value()) engineResources.showColliders = *arguments.showColliders;
	if (!QFE::FRAMEWORK::ASSETS::GetProjectRoot().empty()) {
		engineResources.assetDir = QFE::ConvertString(QFE::FRAMEWORK::ASSETS::GetProjectAssets().wstring()) + "/";
		engineResources.modelDir = engineResources.assetDir;
		engineResources.config.scriptDir = QFE::ConvertString((QFE::FRAMEWORK::ASSETS::GetProjectAssets() / "scripts").wstring());
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
	// エディタから接続先を渡された場合だけ通信し、単独起動時は従来通り動作する。
	namespace Net = QFE::FRAMEWORK::NETWORK;
	std::unique_ptr<QFE::NETWORK::INetworkCore> logNetwork;
	QFE::NETWORK::ConnectHandle logConnection{};
	if (!arguments.logHost.empty() || !arguments.logPort.empty()) {
		logNetwork = Net::CreateNetworkCore();
		if (logNetwork && !arguments.logHost.empty() && !arguments.logPort.empty()) {
			logConnection = Net::Connect(logNetwork.get(), arguments.logHost, arguments.logPort);
		}
		if (logConnection.id == UINT32_MAX) {
			QFE::DebugLog("Failed to connect to the editor log receiver.", QFE::LogLevel::Error);
			logNetwork.reset();
		}
	}
	uint64_t lastLogSequence = 0;
	auto sendRuntimeLogs = [&]() {
		if (!logNetwork) return;
		for (const auto& entry : QFE::MyDebugLog::GetInstance()->GetEntries()) {
			if (entry.sequence <= lastLogSequence) continue;
			const char* level = entry.level == QFE::LogLevel::Error ? "Error" :
				(entry.level == QFE::LogLevel::Warning ? "Warning" : "Info");
			const std::string line = "[" + entry.timestamp + "] [" + level + "] " + entry.message;
			if (!Net::SendString(logNetwork.get(), logConnection, line)) {
				QFE::DebugLog("Failed to send runtime logs to the editor.", QFE::LogLevel::Error);
				logNetwork.reset();
				return;
			}
			lastLogSequence = entry.sequence;
		}
	};
	std::string assetError;
	if (!QFE::FRAMEWORK::ASSETS::OpenProject(*engineSystems.assetManager, QFE::FRAMEWORK::ASSETS::GetProjectRoot(), assetError)) {
		QFE_LOG("Failed to open asset registry: " + assetError, QFE::LogLevel::Error);
		QFE::FRAMEWORK::ShutdownWindowsQuickForgeEngineSystems(engineSystems);
		sendRuntimeLogs();
		return -1;
	}
	QFE::FRAMEWORK::ASSETS::InitializeModelLoader(*engineSystems.modelLoader);
	if (!engineConfigLoaded) QFE::DebugLog("Failed to load EngineConfig.json; using defaults.", QFE::LogLevel::Warning);
	// QuickForgeエンジンの初期化
	QFE::FRAMEWORK::EngineInitialize(engineSystems, engineResources);
	const std::optional<std::filesystem::path> startupScene = ResolveStartupScene(
		engineResources.assetDir,
		arguments.scene);
	if (!startupScene.has_value()) {
		QFE::FRAMEWORK::ShutdownWindowsQuickForgeEngineSystems(engineSystems);
		sendRuntimeLogs();
		return -1;
	}
	// シーンの読み込みはフレームワークに任せ、Runtime がモジュールの操作に依存しないようにする。
	QFE::FRAMEWORK::LoadScene(*engineSystems.sceneManager, QFE::ConvertString(startupScene->wstring()));

	auto& fpsCounter = engineSystems.fpsCounter;
	QFE::SCENE::SceneManager& sceneManager = *engineSystems.sceneManager;
	// EntityManager は Core の型なので、フレームワーク経由で取得した後は直接利用できる。
	QFE::EntityManager& entityManager = QFE::FRAMEWORK::GetSceneEntities(sceneManager);

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

		// エンジンの描画後処理（フレーム終了時の共通処理）
		QFE::FRAMEWORK::EnginePostDraw(engineSystems, engineResources);
		QFE::FRAMEWORK::EndWindowsEngineFrame(engineSystems, &engineResources);
		sendRuntimeLogs();

		// FPSが60fps付近のオブジェクト数をカウント
		if (fpsCounter->GetAverageFPS() > 59.0f && fpsCounter->GetAverageFPS() < 61.0f) {
			objectNums.push_back(static_cast<int>(entityManager.GetActiveEntityIds().size()));
		}
	}

	QFE::FRAMEWORK::ShutdownWindowsQuickForgeEngineSystems(engineSystems);
	sendRuntimeLogs();
	return 0;
}
