#pragma once
#include "../../../../externals/imgui/imgui.h"
#include "../../../../externals/imgui/imgui_impl_dx12.h"
#include "../../../../externals/imgui/imgui_impl_win32.h"

#include <d3d12.h>
#include <wrl.h>
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

class WinApp;
class DirectXCommon;

class ImGuiManager {
public:
	// シングルトン
	static ImGuiManager* GetInstatnce();

public:
	/// <summary>
	/// ImGuiを初期化します
	/// </summary>
	/// <param name="winApp"></param>
	/// <param name="dxCommon"></param>
	void Initialize(WinApp* winApp, DirectXCommon* dxCommon);

private:

private: // シングルトン用
	ImGuiManager() = default;
	~ImGuiManager() = default;
	ImGuiManager(const ImGuiManager&) = delete;
	ImGuiManager& operator=(const ImGuiManager&) = delete;
};