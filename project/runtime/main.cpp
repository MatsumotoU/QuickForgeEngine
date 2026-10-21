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
	void AddSceneCandidates(
		std::vector<std::filesystem::path>& candidates,
		const std::filesystem::path& path) {
		candidates.push_back(path);
		if (path.extension().empty()) {
			std::filesystem::path jsonPath = path;
			jsonPath += ".json";
			candidates.push_back(std::move(jsonPath));
		}
	}

	std::optional<std::string> GetSceneArgument() {
		int argumentCount = 0;
		LPWSTR* arguments = CommandLineToArgvW(GetCommandLineW(), &argumentCount);
		if (arguments == nullptr) return std::nullopt;

		std::optional<std::string> sceneName;
		if (argumentCount >= 2) {
			int sceneArgumentIndex = 1;
			if (std::wstring_view(arguments[1]) == L"--scene") {
				sceneArgumentIndex = 2;
			}
			if (sceneArgumentIndex < argumentCount && arguments[sceneArgumentIndex][0] != L'\0') {
				sceneName = QFE::ConvertString(std::wstring(arguments[sceneArgumentIndex]));
			}
		}
		LocalFree(arguments);
		return sceneName;
	}

	std::optional<std::string> GetConfiguredStartScene(
		const std::filesystem::path& resourceDirectory) {
		const std::filesystem::path gameFlowPath = resourceDirectory / "GameFlow.json";
		nlohmann::json gameFlow;
		if (!QFE::FILE::LoadFileToJson(gameFlowPath.string(), gameFlow)) {
			QFE_LOG(
				"Failed to load game flow: " + gameFlowPath.string(),
				QFE::LogLevel::Error);
			return std::nullopt;
		}

		const auto startScene = gameFlow.find("StartScene");
		if (startScene == gameFlow.end() || !startScene->is_string()) {
			QFE_LOG(
				"GameFlow.json must contain a string named 'StartScene'.",
				QFE::LogLevel::Error);
			return std::nullopt;
		}

		std::string sceneName = startScene->get<std::string>();
		if (sceneName.empty()) {
			QFE_LOG("GameFlow.json contains an empty 'StartScene'.", QFE::LogLevel::Error);
			return std::nullopt;
		}
		return sceneName;
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
		AddSceneCandidates(candidates, requestedPath);
		if (requestedPath.is_relative()) {
			AddSceneCandidates(candidates, resourceDirectory / "scene" / requestedPath);
			AddSceneCandidates(candidates, resourceDirectory / requestedPath);
		}

		for (const std::filesystem::path& candidate : candidates) {
			std::error_code error;
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

	std::vector<int> objectNums;

	// QuickForgeエンジンのリソースを初期化
	QFE::FRAMEWORK::WindowsEngineResources engineResources;
	engineResources.windowName = "ShootingGameRuntime";
	engineResources.windowWidth = 1280;
	engineResources.windowHeight = 720;
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
		gameModule.GetComponentRegistrationFunction())) {
		return -1;
	}
	// QuickForgeエンジンの初期化
	QFE::FRAMEWORK::EngineInitialize(engineSystems, engineResources);
	const std::optional<std::filesystem::path> startupScene = ResolveStartupScene(
		engineResources.assetDir,
		GetSceneArgument());
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
