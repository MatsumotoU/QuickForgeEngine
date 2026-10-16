#pragma once
#include "framework/window/WindowCoreFrameWork.h"
#include "framework/graphic/GraphicEngineCoreFramework.h"
#include "framework/gui/GuiCoreFramework.h"
#include "window/interface/IGameWindowManager.h"
#include "graphics/interface/IGraphicEngine.h"
#include "gui/interface/IGuiManager.h"

namespace QFE::APPLICATION {
	/// @brief このアプリケーションで使うエンジンシステムをまとめるクラス
	class ProjectGeneratorSystemCore final {
	public:
		/// @brief コピーコンストラクタとムーブコンストラクタを禁止する
		ProjectGeneratorSystemCore() = default;
		ProjectGeneratorSystemCore(const ProjectGeneratorSystemCore&) = delete;
		ProjectGeneratorSystemCore& operator=(const ProjectGeneratorSystemCore&) = delete;
		ProjectGeneratorSystemCore(ProjectGeneratorSystemCore&&) = delete;
		ProjectGeneratorSystemCore& operator=(ProjectGeneratorSystemCore&&) = delete;

		std::unique_ptr<QFE::WINDOW::IGameWindowManager> windowManager;
		std::unique_ptr<QFE::GRAPHIC::IGraphicEngine> graphicEngine;
		std::unique_ptr<QFE::GUI::IGuiManager> guiManager;
	};
}
