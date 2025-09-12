#pragma once
#include "Graphic/DirectXCommon/Descriptors/Data/DescriptorHandles.h"
#include "Resources/Shaders/StructsForGpu/OffsetBuffer.h"
#include "Resources/Shaders/StructsForGpu/VignetteOffset.h"
#include "Resources/Shaders/StructsForGpu/ColorCorrectionOffset.h"

#include "Graphic/Pipeline/PSO/PipelineStateObject.h"
#include "Graphic/ShaderBuffer/VertexBuffer.h"
#include "Graphic/ShaderBuffer/ConstantBuffer.h"

#include <d3d12.h>
#include <functional>
#include <vector>
#include <map>
#include <wrl.h>
#include <array>

// TODO: 他の効果を作成する
// TODO: ポストプロセスの順番を動的に変えられるようにする
// TODO: ポストプロセスのシェーダーを動的に作成できるようにする
// TODO: 一つだけ選択したときにシェーダーが適用されないバグを直す

class RendaringPostprosecess {
public:
	RendaringPostprosecess();

public:
	void Initialize(ID3D12Device* device, ID3D12GraphicsCommandList* list);
	void SetColorCorrectionPSO(PipelineStateObject* pso);
	void SetGrayScalePSO(PipelineStateObject* pso);
	void SetVignettePSO(PipelineStateObject* pso);
	void SetNormalPSO(PipelineStateObject* pso);
	void SetOffscreenResource(ID3D12Resource* firstResource, ID3D12Resource* secondResource);
	void SetOffscreenRtvHandle(D3D12_CPU_DESCRIPTOR_HANDLE firstHandle, D3D12_CPU_DESCRIPTOR_HANDLE secondHandle);
	void SetOffscreenSrvHandle(DescriptorHandles firstHandle, DescriptorHandles secondHandle);
	void SetDsvHandle(D3D12_CPU_DESCRIPTOR_HANDLE handle);
	void SetBackBufferRtvHandle(D3D12_CPU_DESCRIPTOR_HANDLE handle);

public:
	void PreDraw();
	void PostDraw();

public:// debug機能
#ifdef _DEBUG
	void DrawImGui();
#endif // _DEBUG

private:
	void ClearFirstRenderTarget();
	void ClearSecondRenderTarget();
	void SwitchRenderTarget();

	void ApplyGrayScale();
	void ApplyVignette();
	void ApplyColorCorrection();

public:// パブリック変数
	bool isPostprocess_;
	bool isImGuiEnabled_;

	bool enableGrayscale_;
	bool enableColorCorrection_;
	bool enableVignette_;
	bool enableNormal_;

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

private:// メンバ変数
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