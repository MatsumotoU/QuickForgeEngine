#pragma once
#include <memory>
#include <unordered_map>
#include "framework/input/InputFrameWork.h"
#include "framework/scene/SceneCoreFramework.h"
#include "framework/scene/AudioComponentSystem.h"
#include "framework/scene/SceneChangeSystem.h"

#include "assetfactory/game-data/AssetManager.h"
#include "assetfactory/model/ModelLoader.h"

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
		std::unique_ptr<QFE::AUDIO::AudioEngine> audioEngine;
		AudioComponentSystem audioComponents;
		// Entity ID が同じサブシーン同士でも音声の所有状態を共有しない。
		std::unordered_map<uint64_t, std::unique_ptr<AudioComponentSystem>> subSceneAudio;
		SceneChangeTransitionSystem sceneChangeTransition;
		std::unique_ptr<QFE::FPSCounter> fpsCounter;// FPSカウンター
		std::unique_ptr<QFE::ASSET::AssetManager> assetManager; ///< プロジェクトのCPU側アセット管理。
		std::unique_ptr<QFE::ASSET::ModelLoader> modelLoader; ///< 読み込み済みモデルを保持する。

		// 所有権を一意に保つため、コピーを禁止する。
		WindowsQuickForgeEngineSystems();
		~WindowsQuickForgeEngineSystems();

		WindowsQuickForgeEngineSystems(const WindowsQuickForgeEngineSystems&) = delete;
		WindowsQuickForgeEngineSystems& operator=(const WindowsQuickForgeEngineSystems&) = delete;
	};
}
