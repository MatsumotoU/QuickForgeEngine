#include "PSOCreater.h"

Microsoft::WRL::ComPtr<ID3D12PipelineState> PSOCreater::CreatePSO(ID3D12Device* device, const D3D12_GRAPHICS_PIPELINE_STATE_DESC& psoDesc) {
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pso;
	HRESULT hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pso));
	if (FAILED(hr)) {
		// エラーハンドリング
		return nullptr;
	}

	// パイプラインステートオブジェクトの生成に成功
	//ShaderCompiler::CompileShaders(psoDesc.VS, psoDesc.PS);

	return pso;
}
