#pragma once
#include <memory>
#include "framework/graphic/D3D12GraphicFrameWork.h"
#include "framework/window/WindowsWindowFrameWork.h"
#include "framework/input/InputFrameWork.h"
#include "framework/gui/D3D12GuiFrameWork.h"
#include "framework/script/WindowsScriptWorkFrame.h"
#include "framework/scene/SceneFrameWork.h"

#include "assetfactory/model/AssimpModelLoader.h"

#include "core/timer/FPSCounter.h"

namespace QFE::FRAMEWORK{
	/// @brief Windowsアプリケーション用のQuickForgeエンジンシステムを保持する構造体。
	class WindowsQuickForgeEngineSystems final {
	public:
		std::unique_ptr<QFE::GameWindowManager> windowManager;// ウィンドウマネージャ
		std::unique_ptr<QFE::GRAPHIC::D3D12GraphicEngine> graphicEngine;// グラフィックエンジン
		std::unique_ptr<QFE::GUI::D3D12GuiManager> guiManager;// GUIマネージャ
		std::unique_ptr<QFE::INPUT::InputInterface> inputInterface;// 入力インターフェース
		std::unique_ptr<QFE::SCRIPT::WindowsScriptInstance> scriptInstance;// スクリプトインスタンス
		std::unique_ptr<QFE::SCENE::SceneManager> sceneManager;// シーンマネージャ
		std::unique_ptr<QFE::FPSCounter> fpsCounter;// FPSカウンター
		std::unique_ptr<QFE::ASSET::AssimpModelLoader> modelLoader;// モデルローダー

		// 所有権を一意に保つため、コピーを禁止する。
		WindowsQuickForgeEngineSystems();
		~WindowsQuickForgeEngineSystems();

		WindowsQuickForgeEngineSystems(const WindowsQuickForgeEngineSystems&) = delete;
		WindowsQuickForgeEngineSystems& operator=(const WindowsQuickForgeEngineSystems&) = delete;
	};
}
