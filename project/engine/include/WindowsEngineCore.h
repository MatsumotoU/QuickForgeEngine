/**
 * @file WindowsEngineCore.h
 * @brief Windows環境におけるエンジンコアの実装
 */

#pragma once
#include "engine/include/core/IEngineCore.h"
#include "engine/include/core/EngineGlobalValue.h"
#include "editor/include/IEditor.h"
#include <Windows.h>
#include <chrono>
#include <nlohmann/json.hpp>

 // Core subsystems
#include "engine/include/window/GameWindowManager.h"
#include "engine/include/graphic/DirectXCommon/DirectXCommon.h"
#include "engine/include/graphic/Pipeline/GraphicPipelineManager.h"
#include "engine/include/utility/DebugTool/App/WinAppDebugCore.h"
#include "engine/include/utility/DebugTool/ImGui/ImGuiFlameController.h"
#include "engine/include/utility/DebugTool/FrameCounter.h"
#include "engine/include/renderer/GraphRenderer.h"
#include "engine/include/graphic/PostEffect/OffScreen/OffScreenResourceManager.h"
#include "engine/include/graphic/PostEffect/RenderingPostprocess.h"
#include "engine/include/assets/AssetManager.h"
#include "engine/include/core/Thread/ThreadPool.h"

// Scene and Scripts
#include "engine/include/scene/SceneManager.h"

// サブモジュール
#include "engine/include/audio/AudioInterface.h"
#include "engine/include/input/InputInterface.h"
#include "engine/include/physics/PhysicsManager.h"
#include "engine/include/collider/ColliderManager.h"
#include "engine/include/utility/MultiThreadTaskExecutor.h"

namespace QFE {

	/**
	 * @class WindowsEngineCore
	 * @brief Windows OS上で動作するエンジンの中心クラス
	 */
	class WindowsEngineCore final : public IEngineCore {
	public:
		/**
		 * @brief コンストラクタ
		 * @param hInstance インスタンスハンドル
		 * @param lpCmdLine コマンドライン引数
		 */
		WindowsEngineCore(HINSTANCE& hInstance, LPSTR& lpCmdLine);
		~WindowsEngineCore() override = default;

		/**
		 * @brief エンジンの初期化
		 */
		void Initialize() override;

		/**
		 * @brief メインループの実行
		 */
		void MainLoop() override;

		/**
		 * @brief エンジンの終了処理
		 */
		void Shutdown() override;

	private:
		void Update();
		void Draw();

		HINSTANCE& hInstance_;
		LPSTR& lpCmdLine_;

		std::unique_ptr<IGameWindowManager> gameWindowManager;
		DirectXCommon* directXCommon_;
		WinAppDebugCore debugCore_;

		OffScreenResourceManager offScreenResourceManager_;

		AssetManager* assetManager_;
		RenderingPostprocess* renderingPostprocess_;
		Render::GraphRenderer* graphRenderer_;

		GraphicPipelineManager* graphicPipelineManager_;
		ImGuiFlameController imguiFrameController_;

		std::unique_ptr<IEditor> editor_ = nullptr;
		FrameCounter frameCounter_;

		InputInterface* inputInterface_;
		SceneManager* sceneManager_;
		PhysicsManager* physicsManager_;
		ColliderManager* colliderManager_;
		IAudioInterface* audioInterface_;
		std::unique_ptr<ThreadPool> threadPool_;

		nlohmann::json configJson_;
	};

}
