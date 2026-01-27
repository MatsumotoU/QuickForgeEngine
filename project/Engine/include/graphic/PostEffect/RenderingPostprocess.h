#pragma once
#include "engine/include/utility/DesignPatterns/Singleton.h"
#include "engine/include/graphic/DirectXCommon/Descriptors/Data/DescriptorHandles.h"
#include "engine/resources/Shaders/ShaderStructs/hlslTypeToCpp.h"

#include "engine/include/graphic/Pipeline/PSO/PipelineStateObject.h"
#include "engine/include/graphic/ShaderBuffer/VertexBuffer.h"
#include "engine/include/graphic/ShaderBuffer/ConstantBuffer.h"

#include <stdint.h>
#include <string>
#include <vector>
#include <list>
#include <unordered_map>
#include <d3d12.h>
#include <functional>
#include <map>
#include <wrl.h>
#include <array>

namespace QFE {
	class DirectXCommon;
	class PipelineStateObject;

	class RenderingPostprocess final : public Singleton<RenderingPostprocess> {
		friend class Singleton<RenderingPostprocess>;

	public:
		RenderingPostprocess();

	public:
		void Initialize(ID3D12Device* device, ID3D12GraphicsCommandList* list);
		void SetColorCorrectionPSO(PipelineStateObject* pso);
		void SetGrayScalePSO(PipelineStateObject* pso);
		void SetVignettePSO(PipelineStateObject* pso);
		void SetNormalPSO(PipelineStateObject* pso);
		void SetPixelPSO(PipelineStateObject* pso);
		void SetOffscreenResource(ID3D12Resource* firstResource, ID3D12Resource* secondResource);
		void SetOffscreenRtvHandle(D3D12_CPU_DESCRIPTOR_HANDLE firstHandle, D3D12_CPU_DESCRIPTOR_HANDLE secondHandle);
		void SetOffscreenSrvHandle(DescriptorHandles firstHandle, DescriptorHandles secondHandle);
		void SetDsvHandle(D3D12_CPU_DESCRIPTOR_HANDLE handle);
		void SetBackBufferRtvHandle(D3D12_CPU_DESCRIPTOR_HANDLE handle);
		DescriptorHandles GetCurrentSrvHandle() const;

		ColorCorrectionOffset& GetColorCorrectionOffset() { return *colorCorrectionOffsetBuffer_.GetData(); }
		OffsetBuffer& GetGrayScaleOffset() { return *grayScaleOffsetBuffer_.GetData(); }
		VignetteOffset& GetVignetteOffset() { return *vignetteOffsetBuffer_.GetData(); }
		PixcelOffset& GetPixelOffset() { return *pixcelOffsetBuffer_.GetData(); }

	public:
		void PreDraw();
		void PostDraw();

	public:// debug機能
#ifdef QFE_OPTIMIZE_OFF
		void DrawImGui();
#endif // QFE_OPTIMIZE_OFF

	private:
		void ClearFirstRenderTarget();
		void ClearSecondRenderTarget();
		void SwitchRenderTarget();

		void ApplyGrayScale();
		void ApplyVignette();
		void ApplyColorCorrection();
		void ApplyPixcel();

	public:// パブリック変数
		bool isPostprocess_;
		bool isImGuiEnabled_;

		bool enableGrayscale_;
		bool enableColorCorrection_;
		bool enableVignette_;
		bool enableNormal_;
		bool enablePixcel_;

	private:// 色調補正
		PipelineStateObject* colorCorrectionPso_;
		ConstantBuffer<ColorCorrectionOffset> colorCorrectionOffsetBuffer_;
		int colorCorrectionProcessIndex_;

	private:// グレースケール変数
		PipelineStateObject* grayScalePso_;
		ConstantBuffer<OffsetBuffer> grayScaleOffsetBuffer_;
		float grayScaleOffset_;
		int grayScaleProcessIndex_;

	private:// ビネット変数
		PipelineStateObject* vignettePso_;
		ConstantBuffer<VignetteOffset> vignetteOffsetBuffer_;
		int vignetteProcessIndex_;

	private:// ピクセル化変数
		PipelineStateObject* pixcelPso_;
		ConstantBuffer<PixcelOffset> pixcelOffsetBuffer_;
		int pixcelProcessIndex_;

	private:// メンバ変数
		DirectXCommon* dxCommon_ = nullptr;
		ID3D12Device* device_;
		ID3D12GraphicsCommandList* list_;
		PipelineStateObject* normalPso_;
		std::array<ID3D12Resource*, 2> offScreenResources_;
		std::vector<std::function<void() >> postProcessFunctions_;
		std::vector<uint32_t> postProcessOrderForm_;

		D3D12_CPU_DESCRIPTOR_HANDLE backBufferRtvHandle_;
		std::array<D3D12_CPU_DESCRIPTOR_HANDLE, 2> offScreenRtvHandles_;
		std::array<DescriptorHandles, 2> offScreenSrvHandles_;
		D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle_;

		uint32_t postProcessCount_;
		uint32_t renderingRosourceIndex_;
		uint32_t readingResourceIndex_;
		bool isFirstStateRenderTarget_;
		bool isSecondStateRenderTarget_;

	private:// 画面用
		float offScreenClearColor[4];
		Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;
		D3D12_VERTEX_BUFFER_VIEW vertexBufferView_;
		VertexData* vertexData_;
		Microsoft::WRL::ComPtr<ID3D12Resource> indexResource_;
		D3D12_INDEX_BUFFER_VIEW indexBufferView_;
		uint32_t* indexData_;
	};
}
