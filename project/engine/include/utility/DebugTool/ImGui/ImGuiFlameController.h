#pragma once
#define IMGUI_ENEBLE_VIEWPORTS
#define IMGUI_HAS_DOCK

#include <d3d12.h>
#include <wrl.h>
#include <vector>

#include "ImGuiInclude.h"
namespace QFE {
	class ImGuiFlameController final {
	public:
		ImGuiFlameController();
		~ImGuiFlameController();
	public:
		/// <summary>
		/// ImGui繧抵ｿｽE譛溷喧縺励∪縺・
		/// </summary>
		/// <param name="winApp"></param>
		/// <param name="dxCommon"></param>
		void Initialize(const HWND& hwnd, ID3D12GraphicsCommandList* commandList, ID3D12DescriptorHeap* const* srvDescriptorHeap);
		/// <summary>
		/// ImGui繧堤ｵゆｺ・・ｽ・ｽ縺ｾ縺吶ゅご繝ｼ繝繝ｫ繝ｼ繝励′邨ゅｏ縺｣縺溷ｾ後↓鄂ｮ縺阪∪縺・
		/// </summary>
		void EndImGui();
		/// <summary>
		/// 縺薙％縺九ｉImGui繧剃ｽｿ縺医ｋ
		/// </summary>
		void BeginFrame();
		/// <summary>
		/// ImGui縺薙％縺ｾ縺ｧ縺難ｿｽE髢｢謨ｰ縺ｯ繧ｷ繝ｼ繝ｫ繝峨ｒ陦ｨ遉ｺ逕ｨ縺ｫ蛻・・ｽ・ｽ譖ｿ縺医ｋ蜑阪↓縺吶ｋ
		/// </summary>
		void EndFrame(D3D12_CPU_DESCRIPTOR_HANDLE currentBackBufferCpuHandle);

	private:
		int stateCheck_;
		ID3D12GraphicsCommandList* commandList_;
		ID3D12DescriptorHeap* const* srvDescriptorHeap_;
	};
}

