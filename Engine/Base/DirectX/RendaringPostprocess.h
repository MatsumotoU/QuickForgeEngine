#pragma once
class EngineCore;
#include "Shaders/StructsForGpu/OffsetBuffer.h"
#include "Shaders/StructsForGpu/VignetteOffset.h"
#include "Shaders/StructsForGpu/ColorCorrectionOffset.h"

#include "Base/DirectX/PipelineStateObject.h"
#include "Base/DirectX/Resource/ShaderBuffers/VertexBuffer.h"
#include "Base/DirectX/Resource/ShaderBuffers/ConstantBuffer.h"

#include <functional>
#include <vector>
#include <map>
#include <wrl.h>

// TODO: 他の効果を作成する
// TODO: ポストプロセスの順番を動的に変えられるようにする
// TODO: ポストプロセスのシェーダーを動的に作成できるようにする
// TODO: 一つだけ選択したときにシェーダーが適用されないバグを直す

class RendaringPostprosecess {
public:
	RendaringPostprosecess();

public:
	void Initialize(EngineCore* engineCore);

public:
	void PreDraw();
	void PostDraw();

public:// debug機能
#ifdef _DEBUG
	void DrawImGui();
#endif // _DEBUG

public:
	D3D12_GPU_DESCRIPTOR_HANDLE GetOffscreenSrvHandleGPU();

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
	std::vector<std::function<void() >> postProcessFunctions_;
	std::vector<uint32_t> postProcessOrderForm_;

	EngineCore* engineCore_;

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