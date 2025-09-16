#pragma once
#define IMGUI_ENEBLE_VIEWPORTS
#define IMGUI_HAS_DOCK

#include <d3d12.h>
#include <wrl.h>
#include <vector>

#include "../ImGuiInclude.h"

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
	void EndFrame(D3D12_CPU_DESCRIPTOR_HANDLE currentBackBufferCpuHandle);

private:
#ifdef _DEBUG
	int stateCheck_;
#endif // _DEBUG
	ID3D12GraphicsCommandList* commandList_;
	ID3D12DescriptorHeap* const* srvDescriptorHeap_;
};