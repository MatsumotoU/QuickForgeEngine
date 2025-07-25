#pragma once
#include <d3d12.h>
#include <wrl.h>

class PSO {
private:
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState_;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
};