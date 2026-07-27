#define NOMINMAX
#include <Windows.h>

#include "gamesystem/ShootingSystem.h"

#include "framework/application/WindowsEngineFramework.h"
#include "framework/application/WindowsEngineResources.h"

#include "window/GameWindowManager.h"
#include "graphics/D3D12GraphicEngine.h"
#include "gui/D3D12GuiManager.h"

#include "camera/CameraManager.h"
#include "scene/SceneManager.h"
#include "components/AllComponent.h"
#include "core/logger/MyDebugLog.h"
#include "core/string/MyString.h"
#include "core/timer/FPSCounter.h"
#include "script/ScriptInstance.h"
#include "script/EngineContextForScript.h"
#include "input/InputInterface.h"

#include "assetfactory/model/AssimpModelLoader.h"

#include "core/math/transform/Transform.h"

#include <imgui.h>

#include <array>

#include "EngineDefines.h"

#include "../resources/shaders/shaderStructs/hlslTypeToCpp.h"

/// /// @brief Windowsアプリケーションのテスト
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {

	std::vector<int> objectNums;

	// QuickForgeエンジンのリソースを初期化
	QFE::FRAMEWORK::WindowsEngineResources engineResources;
	engineResources.windowName = "ShootingGameRuntime";
	engineResources.windowWidth = 1280;
	engineResources.windowHeight = 720;
	// QuickForgeエンジンのシステムを生成
	QFE::FRAMEWORK::WindowsQuickForgeEngineSystems engineSystems;
	if (!QFE::FRAMEWORK::CreateWindowsQuickForgeEngineSystems(
		hInstance,
		engineResources.windowName,
		engineResources.windowWidth, engineResources.windowHeight,
		engineSystems)) {
		return -1;
	}
	// QuickForgeエンジンの初期化
	QFE::FRAMEWORK::EngineInitialize(engineSystems, engineResources);
	auto& gameWindowManager = engineSystems.windowManager;
	auto& graphicEngine = engineSystems.graphicEngine;
	auto& guiManager = engineSystems.guiManager;
	auto& inputInterface = engineSystems.inputInterface;
	auto& fpsCounter = engineSystems.fpsCounter;
	QFE::SCENE::SceneManager& sceneManager = *engineSystems.sceneManager;
	QFE::EntityManager& entityManager = sceneManager.GetCurrentSceneEntityManager();

	// メインループ
	while (QFE::FRAMEWORK::IsMainWindowActive(engineSystems.windowManager.get())) {
		if (!QFE::FRAMEWORK::ProcessWindowsApplicationMessage()) {
			break;
		}
		if (!QFE::FRAMEWORK::BeginWindowsEngineFrame(engineSystems)) {
			break;
		}

		float deltaTime = fpsCounter->GetDeltaTime();

		// AutoScroll
		QFE::GAMESYSTEM::AutoScrollSystem(engineSystems, engineResources, deltaTime);
		// シューティングプレイヤーの実行
		QFE::GAMESYSTEM::ShootingPlayerSystem(engineSystems, engineResources, deltaTime);
		// 敵の処理
		QFE::GAMESYSTEM::ShootingEnemySystem(engineSystems, engineResources, deltaTime);
		// 弾丸の処理
		QFE::GAMESYSTEM::BulletUpdateSystem(engineSystems, engineResources, deltaTime);
		// コライダーの処理
		QFE::GAMESYSTEM::CollisionSystem(engineSystems, engineResources, deltaTime);
		// 移動制限コンポーネントの処理
		QFE::GAMESYSTEM::MoveLimitSystem(engineSystems, engineResources, deltaTime);
		// プレイヤー自動トラッキング処理
		QFE::GAMESYSTEM::PlayerTrackingSystem(engineSystems, engineResources, deltaTime);
		// 敵のスポーン処理
		QFE::GAMESYSTEM::EnemySpawnerSystem(engineSystems, engineResources, deltaTime);

		// カメラの更新
		engineResources.viewProj = QFE::FRAMEWORK::UpdateMainCamera(sceneManager);

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
