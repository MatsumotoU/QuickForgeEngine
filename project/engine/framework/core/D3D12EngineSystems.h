#pragma once
#include <memory>
#include <string>
#include "framework/graphic/D3D12GraphicFrameWork.h"
#include "framework/window/WindowsWindowFrameWork.h"
#include "framework/input/InputFrameWork.h"
#include "framework/scene/SceneFrameWork.h"

namespace QFE {
	namespace FRAMEWORK {
		/// @brief D3D12用のエンジンシステムを所持するクラス
		class D3D12EngineSystems {
		public:
			std::unique_ptr<QFE::GRAPHIC::D3D12GraphicEngine> graphicEngine_;
			std::unique_ptr<QFE::INPUT::InputInterface> inputInterface_;
			std::unique_ptr<QFE::SCENE::SceneManager> sceneManager_;
			std::unique_ptr<QFE::GameWindowManager> gameWindowManager_;
		};
	}
}
