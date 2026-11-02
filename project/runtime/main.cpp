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
#include "assetfactory/game-data/GamePak.h"
#include <commdlg.h>
#include <random>
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

	// pak とシーンを別々に指定できる。従来の単独シーン引数も受け付ける。
	struct Arguments { std::optional<std::string> scene; std::filesystem::path pak; };
	Arguments GetArguments() {
		Arguments result; int count = 0;
		LPWSTR* args = CommandLineToArgvW(GetCommandLineW(), &count);
		if (!args) return result;
		for (int i = 1; i < count; ++i) {
			const std::wstring_view arg(args[i]);
			if (arg == L"--pak" && i + 1 < count) result.pak = args[++i];
			else if (arg == L"--scene" && i + 1 < count) result.scene = QFE::ConvertString(std::wstring(args[++i]));
			else if (!arg.starts_with(L"--")) {
				const std::filesystem::path path(args[i]);
				if (_wcsicmp(path.extension().c_str(), L".qfepak") == 0 && result.pak.empty()) result.pak = path;
				else if (!result.scene) result.scene = QFE::ConvertString(std::wstring(args[i]));
			}
		}
		LocalFree(args); return result;
	}

	std::filesystem::path ChoosePackage() {
		std::array<wchar_t, 32768> path{};
		OPENFILENAMEW dialog{};
		dialog.lStructSize = sizeof(dialog);
		dialog.lpstrFilter = L"QuickForge package (*.qfepak)\0*.qfepak\0\0";
		dialog.lpstrFile = path.data(); dialog.nMaxFile = static_cast<DWORD>(path.size());
		dialog.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
		return GetOpenFileNameW(&dialog) ? std::filesystem::path(path.data()) : std::filesystem::path{};
	}

	// 既存ローダーはファイルパスを要求するため、pak を専用の一時ディレクトリへ展開する。
	// 所有するディレクトリだけを、エンジン終了後に削除する。
	class TemporaryPackage final {
	public:
		TemporaryPackage() = default;
		TemporaryPackage(const TemporaryPackage&) = delete;
		TemporaryPackage& operator=(const TemporaryPackage&) = delete;
		~TemporaryPackage() { if (!root_.empty()) { std::error_code error; std::filesystem::remove_all(root_, error); } }
		bool Open(const std::filesystem::path& pak, std::string& message) {
			std::error_code error;
			const auto base = std::filesystem::temp_directory_path(error);
			if (error) { message = "Could not find a temporary directory."; return false; }
			std::random_device random;
			for (unsigned i = 0; i < 32; ++i) {
				const auto candidate = base / (L"qfe-pak-" + std::to_wstring(GetCurrentProcessId()) + L"-" + std::to_wstring(random()));
				if (std::filesystem::create_directory(candidate, error)) { root_ = candidate; break; }
				error.clear();
			}
			if (root_.empty()) { message = "Could not create a temporary directory."; return false; }
			return QFE::PAK::Extract(pak, root_, message);
		}
		const std::filesystem::path& Root() const { return root_; }
	private:
		std::filesystem::path root_;
	};

	std::optional<std::string> GetConfiguredStartScene(
		const std::filesystem::path& resourceDirectory) {
		(void)resourceDirectory;
		std::ifstream file(QFE::PROJECT::Root() / "project.json");
		try {
			const auto json = nlohmann::json::parse(file);
			const auto scene = json.value("start_scene", std::string{});
			if (!scene.empty()) return scene;
		} catch (const nlohmann::json::exception&) {}
		QFE_LOG("The package does not contain a valid start_scene.", QFE::LogLevel::Error);
		return std::nullopt;
	}

	std::optional<std::filesystem::path> ResolveStartupScene(
		const std::filesystem::path& resourceDirectory,
		const std::optional<std::string>& commandLineScene) {
		const std::optional<std::string> sceneName = commandLineScene.has_value()
			? commandLineScene
			: GetConfiguredStartScene(resourceDirectory);
		if (!sceneName.has_value()) return std::nullopt;

		const std::filesystem::path requestedPath(*sceneName);
		std::vector<std::filesystem::path> candidates;
		if (requestedPath.is_absolute() || QFE::PROJECT::Root().empty()) AddSceneCandidates(candidates, requestedPath);
		if (requestedPath.is_relative()) {
			// 旧形式の resources/scene/... も pak 内の assets/scenes/... として扱う。
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
	const auto pak = arguments.pak.empty() ? ChoosePackage() : arguments.pak;
	if (pak.empty()) return 0;
	// コマンドラインの相対パスは、エンジンの作業ディレクトリ変更前に解決する。
	std::error_code packagePathError;
	const auto absolutePak = std::filesystem::absolute(pak, packagePathError);
	if (packagePathError) { MessageBoxW(nullptr, L"Invalid package path.", L"QuickForge Runtime", MB_OK | MB_ICONERROR); return -1; }
	SetEngineWorkingDirectory();
	TemporaryPackage package;
	std::string packageError;
	if (!package.Open(absolutePak, packageError)) {
		MessageBoxW(nullptr, QFE::ConvertString(packageError).c_str(), L"QuickForge Runtime", MB_OK | MB_ICONERROR);
		return -1;
	}
	QFE::PROJECT::SetRoot(package.Root());
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
	engineSystems.sceneManager->LoadCurrentSceneFromJson(startupScene->string());

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
		QFE::FRAMEWORK::EndWindowsEngineFrame(engineSystems);

		// FPSが60fps付近のオブジェクト数をカウント
		if (fpsCounter->GetAverageFPS() > 59.0f && fpsCounter->GetAverageFPS() < 61.0f) {
			objectNums.push_back(static_cast<int>(entityManager.GetActiveEntityIds().size()));
		}
	}

	QFE::FRAMEWORK::ShutdownWindowsQuickForgeEngineSystems(engineSystems);
	return 0;
}
