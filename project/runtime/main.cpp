#define NOMINMAX
#include <Windows.h>

#include "framework/application/WindowsEngineFramework.h"
#include "framework/application/WindowsGameModule.h"
#include "framework/window/WindowCoreFrameWork.h"
#include "framework/application/WindowsEngineResources.h"


#include "scene/SceneManager.h"
#include "core/logger/MyDebugLog.h"
#include "core/string/MyString.h"
#include "core/timer/FPSCounter.h"
#include "input/interface/InputInterface.h"

#include "assetfactory/model/AssimpModelLoader.h"

#include "core/math/transform/Transform.h"

#include <imgui.h>

#include <array>

#include "EngineDefines.h"

#include "resources/shaders/shaderStructs/hlslTypeToCpp.h"

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
