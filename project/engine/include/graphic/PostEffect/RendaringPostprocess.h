#pragma once
#include "engine/include/utility/DesignPatterns/Singleton.h"
#include "engine/include/graphic/DirectXCommon/Descriptors/Data/DescriptorHandles.h"
#include "Resources/Shaders/ShaderStructs/hlslTypeToCpp.h"

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

// TODO: 莉悶・蜉ｹ譫懊ｒ菴懈・縺吶ｋ
// TODO: 繝昴せ繝医・繝ｭ繧ｻ繧ｹ縺ｮ鬆・分繧貞虚逧・↓螟峨∴繧峨ｌ繧九ｈ縺・↓縺吶ｋ
// TODO: 繝昴せ繝医・繝ｭ繧ｻ繧ｹ縺ｮ繧ｷ繧ｧ繝ｼ繝繝ｼ繧貞虚逧・↓菴懈・縺ｧ縺阪ｋ繧医≧縺ｫ縺吶ｋ
// TODO: 荳縺､縺縺鷹∈謚槭＠縺溘→縺阪↓繧ｷ繧ｧ繝ｼ繝繝ｼ縺碁←逕ｨ縺輔ｌ縺ｪ縺・ヰ繧ｰ繧堤峩縺・

class DirectXCommon;

class RendaringPostprosecess final : public Singleton<RendaringPostprosecess> {
	friend class Singleton<RendaringPostprosecess>;

public:
	RendaringPostprosecess();

public:
	void Initialize(ID3D12Device* device, ID3D12GraphicsCommandList* list);
	void SetColorCorrectionPSO(PipelineStateObject* pso);
	void SetGrayScalePSO(PipelineStateObject* pso);
	void SetVignettePSO(PipelineStateObject* pso);
	void SetNormalPSO(PipelineStateObject* pso);
	void SetPixcelPSO(PipelineStateObject* pso);
	void SetOffscreenResource(ID3D12Resource* firstResource, ID3D12Resource* secondResource);
	void SetOffscreenRtvHandle(D3D12_CPU_DESCRIPTOR_HANDLE firstHandle, D3D12_CPU_DESCRIPTOR_HANDLE secondHandle);
	void SetOffscreenSrvHandle(DescriptorHandles firstHandle, DescriptorHandles secondHandle);
	void SetDsvHandle(D3D12_CPU_DESCRIPTOR_HANDLE handle);
	void SetBackBufferRtvHandle(D3D12_CPU_DESCRIPTOR_HANDLE handle);
	DescriptorHandles GetCurrentSrvHandle() const;

	ColorCorrectionOffset& GetColorCorrectionOffset() { return *colorCorrectionOffsetBuffer_.GetData(); }
	OffsetBuffer& GetGrayScaleOffset() { return *grayScaleOffsetBuffer_.GetData(); }
	VignetteOffset& GetVignetteOffset() { return *vignetteOffsetBuffer_.GetData(); }
	PixcelOffset& GetPixcelOffset() { return *pixcelOffsetBuffer_.GetData(); }

public:
	void PreDraw();
	void PostDraw();

public:// debug讖溯・
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
	void ApplyPixcel();

public:// 繝代ヶ繝ｪ繝・け螟画焚
	bool isPostprocess_;
	bool isImGuiEnabled_;

	bool enableGrayscale_;
	bool enableColorCorrection_;
	bool enableVignette_;
	bool enableNormal_;
	bool enablePixcel_;

private:// 濶ｲ隱ｿ陬懈ｭ｣
	PipelineStateObject* colorCorrectionPso_;
	ConstantBuffer<ColorCorrectionOffset> colorCorrectionOffsetBuffer_;
	int colorCorrectionProcessIndex_;

private:// 繧ｰ繝ｬ繝ｼ繧ｹ繧ｱ繝ｼ繝ｫ螟画焚
	PipelineStateObject* grayScalePso_;
	ConstantBuffer<OffsetBuffer> grayScaleOffsetBuffer_;
	float grayScaleOffset_;
	int grayScaleProcessIndex_;

private:// 繝薙ロ繝・ヨ螟画焚
	PipelineStateObject* vignettePso_;
	ConstantBuffer<VignetteOffset> vignetteOffsetBuffer_;
	int vignetteProcessIndex_;

private:// 繝斐け繧ｻ繝ｫ蛹門､画焚
	PipelineStateObject* pixcelPso_;
	ConstantBuffer<PixcelOffset> pixcelOffsetBuffer_;
	int pixcelProcessIndex_;

private:// 繝｡繝ｳ繝仙､画焚
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

private:// 逕ｻ髱｢逕ｨ
	float offScreenClearColor[4];
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_;
	VertexData* vertexData_;
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource_;
	D3D12_INDEX_BUFFER_VIEW indexBufferView_;
	uint32_t* indexData_;
};
