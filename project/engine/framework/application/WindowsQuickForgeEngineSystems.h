#pragma once
#include <memory>
#include "framework/input/InputFrameWork.h"
#include "framework/scene/SceneFrameWork.h"
#include "framework/scene/AudioComponentSystem.h"
#include "framework/scene/SceneChangeSystem.h"

#include "assetfactory/model/AssimpModelLoader.h"

#include "core/timer/FPSCounter.h"

namespace QFE::WINDOW {
	class IGameWindowManager;
}
namespace QFE::GRAPHIC {
	class IGraphicEngine;
}
namespace QFE::GUI {
	class IGuiManager;
}
namespace QFE {
	class StaticComponentRegistry;
}

namespace QFE::FRAMEWORK{
	class LuaScriptSystem;

	/// @brief Windowsアプリケーション用のQuickForgeエンジンシステムを保持する構造体。
	class WindowsQuickForgeEngineSystems final {
	public:
		// SceneManagerより後まで生存し、各EntityManagerからはweak_ptrで参照される。
		std::shared_ptr<QFE::StaticComponentRegistry> componentRegistry;
		std::unique_ptr<QFE::WINDOW::IGameWindowManager> windowManager;// ウィンドウマネージャ
		std::unique_ptr<QFE::GRAPHIC::IGraphicEngine> graphicEngine;// グラフィック機能の公開インターフェース
		std::unique_ptr<QFE::GUI::IGuiManager> guiManager;// GUIマネージャ
		std::unique_ptr<QFE::INPUT::InputInterface> inputInterface;// 入力インターフェース
		std::unique_ptr<QFE::SCENE::SceneManager> sceneManager;// シーンマネージャ
		std::unique_ptr<LuaScriptSystem> luaScriptSystem;// Luaゲームロジック
		std::unique_ptr<QFE::AUDIO::AudioEngine> audioEngine;
		AudioComponentSystem audioComponents;
		SceneChangeTransitionSystem sceneChangeTransition;
		std::unique_ptr<QFE::FPSCounter> fpsCounter;// FPSカウンター
		std::unique_ptr<QFE::ASSET::AssimpModelLoader> modelLoader;// モデルローダー

		// 所有権を一意に保つため、コピーを禁止する。
		WindowsQuickForgeEngineSystems();
		~WindowsQuickForgeEngineSystems();

		WindowsQuickForgeEngineSystems(const WindowsQuickForgeEngineSystems&) = delete;
		WindowsQuickForgeEngineSystems& operator=(const WindowsQuickForgeEngineSystems&) = delete;
	};
}
