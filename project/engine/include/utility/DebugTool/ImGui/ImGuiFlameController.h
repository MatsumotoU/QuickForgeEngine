#pragma once
#define IMGUI_ENEBLE_VIEWPORTS
#define IMGUI_HAS_DOCK

#include <d3d12.h>
#include <wrl.h>
#include <vector>

#include "ImGuiInclude.h"

class ImGuiFlameController final {
public:
	ImGuiFlameController();
	~ImGuiFlameController();
public:
	/// <summary>
	/// ImGuiを�E期化しまぁE
	/// </summary>
	/// <param name="winApp"></param>
	/// <param name="dxCommon"></param>
	void Initialize(const HWND& hwnd, ID3D12GraphicsCommandList* commandList, ID3D12DescriptorHeap* const* srvDescriptorHeap);
	/// <summary>
	/// ImGuiを終亁E��ます。ゲームループが終わった後に置きまぁE
	/// </summary>
	void EndImGui();
	/// <summary>
	/// ここからImGuiを使える
	/// </summary>
	void BeginFrame();
	/// <summary>
	/// ImGuiここまでこ�E関数はシールドを表示用に刁E��替える前にする
	/// </summary>
	void EndFrame(D3D12_CPU_DESCRIPTOR_HANDLE currentBackBufferCpuHandle);

private:
#ifdef _DEBUG
	int stateCheck_;
#endif // _DEBUG
	ID3D12GraphicsCommandList* commandList_;
	ID3D12DescriptorHeap* const* srvDescriptorHeap_;
};
