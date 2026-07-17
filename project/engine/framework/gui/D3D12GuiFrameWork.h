#pragma once
#define NOMINMAX
#include <Windows.h>
#include <memory>
#include <imgui/imgui.h>

namespace QFE::GUI {
	class D3D12GuiManager;
}
namespace QFE::GRAPHIC {
	class D3D12GraphicEngine;
	enum class DirectXResourceHandle : uint32_t;
}

namespace QFE::FRAMEWORK {
	/// @brief GUIマネージャを生成する関数.グラフィックエンジンとウィンドウのハンドルを引数に取ります.
	std::unique_ptr<QFE::GUI::D3D12GuiManager> CreateGuiManager(
		QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine, HWND hwnd);
	/// @brief DirectXResourceHandleからImGuiのテクスチャIDを取得する関数.グラフィックエンジンとリソースハンドルを引数に取ります.
	ImTextureID GetImGuiTextureIdFromResourceHandle(
		QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine, QFE::GRAPHIC::DirectXResourceHandle resourceHandle);
}