#pragma once
#define NOMINMAX
#include <Windows.h>
#include <memory>

namespace QFE::GUI {
	class D3D12GuiManager;
}
namespace QFE::GRAPHIC {
	class D3D12GraphicEngine;
}

namespace QFE::FRAMEWORK {
	/// @brief GUIマネージャを生成する関数.グラフィックエンジンとウィンドウのハンドルを引数に取ります.
	std::unique_ptr<QFE::GUI::D3D12GuiManager> CreateGuiManager(
		QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine, HWND hwnd);
}