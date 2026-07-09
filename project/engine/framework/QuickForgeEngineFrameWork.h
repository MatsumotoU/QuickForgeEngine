#pragma once
#include "D3D12GraphicFrameWork.h"
#include "SceneFrameWork.h"
#include "WindowsWindowFrameWork.h"

namespace QFE::FRAMEWORK {
	/// @brief エンジンの初期時に使う設定
	struct EngineConfigDesc {
		std::string mainWindowName;
		uint32_t mainWindowWidth;
		uint32_t mainWindowHeight;
	};

	/// @brief windows用のエンジンシステムたち
	struct WindowsEngineSystems {
		std::unique_ptr<QFE::GRAPHIC::D3D12GraphicEngine> graphicEngine;
		std::unique_ptr<QFE::GameWindowManager> windowManager;
	};

	/// @brief windows用のエンジン設定を構築します
	bool InitializeEngine(WindowsEngineSystems& windowsEngineSystems,const EngineConfigDesc& config);
	/// @brief windows用のエンジンをシャットダウンします
	bool FinalizeEngine(WindowsEngineSystems& windowsEngineSystems);
}
