#define NOMINMAX
#include <Windows.h>
#include <shellapi.h>

#include <filesystem>
#include <string_view>

#include "ProjectGeneratorSystemCore.h"

#include "ProjectGenerator.h"

/// @brief アプリケーションのエントリーポイント
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	int argumentCount = 0;
	LPWSTR* arguments = CommandLineToArgvW(GetCommandLineW(), &argumentCount);
	if (arguments != nullptr && argumentCount >= 3 &&
		std::wstring_view(arguments[1]) == L"--generate-config") {
		QFE::APPLICATION::ProjectGenerator batchGenerator;
		const bool generated = batchGenerator.GenerateFromConfiguration(
			std::filesystem::path(arguments[2]));
		LocalFree(arguments);
		return generated ? 0 : 1;
	}
	if (arguments != nullptr) {
		LocalFree(arguments);
	}

	// エンジンの機能生成
	QFE::APPLICATION::ProjectGeneratorSystemCore engineSystems;
	engineSystems.windowManager = QFE::FRAMEWORK::CreateWindowManager(
		"Project Generator", 1280, 720);
	QFE::FRAMEWORK::GraphicEngineFeatureState featureState;
	engineSystems.graphicEngine = QFE::FRAMEWORK::CreateGraphicEngine(
		*engineSystems.windowManager,
		featureState);
	engineSystems.guiManager =
		QFE::FRAMEWORK::CreateGuiManager(
			*engineSystems.graphicEngine, *engineSystems.windowManager);

	QFE::APPLICATION::ProjectGenerator projectGenerator;
	projectGenerator.Initialize();

	// Main loop.
	while (QFE::FRAMEWORK::CanLoopWindowManager(*engineSystems.windowManager)) {
		// アプリケーション更新
		projectGenerator.Update();

		// 描画前処理
		QFE::FRAMEWORK::PreDrawGraphicEngine(
			engineSystems.graphicEngine.get());
		QFE::FRAMEWORK::PreDrawGui(*engineSystems.guiManager);

		// アプリケーション描画
		projectGenerator.Draw(engineSystems);

		// 描画先をスワップチェーンに設定
		QFE::FRAMEWORK::SetSwapChainRenderTarget(
			*engineSystems.graphicEngine);

		// 描画後処理
		QFE::FRAMEWORK::PostDrawGui(
			*engineSystems.guiManager);
		QFE::FRAMEWORK::PostDrawGraphicEngine(
			engineSystems.graphicEngine.get());
	}

	// Shutdown.
	projectGenerator.Shutdown();
	QFE::FRAMEWORK::ShutdownGui(
		*engineSystems.guiManager);
	QFE::FRAMEWORK::ShutdownGraphicEngine(
		engineSystems.graphicEngine.get());
	return 0;
}
