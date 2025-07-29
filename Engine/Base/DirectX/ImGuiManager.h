#pragma once
#define IMGUI_ENEBLE_VIEWPORTS
#define IMGUI_HAS_DOCK
// 基本機能
#include "../../../externals/imgui/imgui.h"
#include "../../../externals/imgui/imgui_impl_dx12.h"
#include "../../../externals/imgui/imgui_impl_win32.h"
// トグル
#include "../../../externals/imgui/imgui_toggle-main/imgui_toggle.h"// 一部警告無視
// グラフ
#include "../../../externals/imgui/implot-master/implot.h"
// ノブ
#include "../../../externals/imgui/imgui-knobs-main/imgui-knobs.h"// かなりの数の警告無視(imgui-knobs.cpp要参照)
// ギズモ
#include "../externals/imgui/ImGuizmo-1.83/ImGuizmo.h"
// ノードエディタ
#include "Base/DirectX/ImGuiNode.h"

#include <d3d12.h>
#include <wrl.h>
#include <vector>
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

class WinApp;
class DirectXCommon;
#ifdef _DEBUG
class MyDebugLog;
#endif // _DEBUG

class ImGuiManager final {
public:
	ImGuiManager();
	~ImGuiManager();
public:
	/// <summary>
	/// ImGuiを初期化します
	/// </summary>
	/// <param name="winApp"></param>
	/// <param name="dxCommon"></param>
	void Initialize(WinApp* winApp, DirectXCommon* dxCommon, ID3D12DescriptorHeap* srvDescriptorHeap);
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

public:
	ID3D12DescriptorHeap* GetSrvDescriptorHeap();

private:
	ID3D12DescriptorHeap* srvDescriptorHeap_;
	Microsoft::WRL::ComPtr<ID3D12Device> device_;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList_;

private:
#ifdef _DEBUG
	int stateCheck_;
	MyDebugLog* debugLog_;
#endif // _DEBUG

};