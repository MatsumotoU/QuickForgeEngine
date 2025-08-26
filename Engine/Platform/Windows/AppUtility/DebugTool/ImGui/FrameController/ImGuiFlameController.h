#pragma once
#define IMGUI_ENEBLE_VIEWPORTS
#define IMGUI_HAS_DOCK

#include <d3d12.h>
#include <wrl.h>
#include <vector>

#include "Externals/imgui/imgui.h"
#include "Externals/imgui/imgui_impl_dx12.h"
#include "Externals/imgui/imgui_impl_win32.h"

#include "Externals/imgui/imgui_toggle-main/imgui_toggle.h"// 一部警告無視
#include "Externals/imgui/implot-master/implot.h"
#include "Externals/imgui/imgui-knobs-main/imgui-knobs.h"// かなりの数の警告無視(imgui-knobs.cpp要参照)
#include "Externals/imgui/ImGuizmo-1.83/ImGuizmo.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

class ImGuiFlameController final {
public:
	ImGuiFlameController();
	~ImGuiFlameController();
public:
	/// <summary>
	/// ImGuiを初期化します
	/// </summary>
	/// <param name="winApp"></param>
	/// <param name="dxCommon"></param>
	void Initialize(const HWND& hwnd, ID3D12GraphicsCommandList* commandList, ID3D12DescriptorHeap* const* srvDescriptorHeap);
	/// <summary>
	/// ImGuiを終了します。ゲームループが終わった後に置きます
	/// </summary>
	void EndImGui();
	/// <summary>
	/// ここからImGuiを使える
	/// </summary>
	void BeginFrame();
	/// <summary>
	/// ImGuiここまでこの関数はシールドを表示用に切り替える前にする
	/// </summary>
	void EndFrame();

private:
#ifdef _DEBUG
	int stateCheck_;
#endif // _DEBUG
	ID3D12GraphicsCommandList* commandList_;
	ID3D12DescriptorHeap* const* srvDescriptorHeap_;
};