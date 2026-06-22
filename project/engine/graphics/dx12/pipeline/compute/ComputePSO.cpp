#include "ComputePSO.h"
#include "EngineDefines.h"

using namespace QFE::GRAPHIC;

void ComputePSO::CreatePipelineStateObject(
	IDxcBlob* csBlob, const D3D12_ROOT_SIGNATURE_DESC& rootSigDesc, ID3D12Device* device) {

    // --- 1. ルートシグネチャの生成 ---
    Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;

    HRESULT hr = D3D12SerializeRootSignature(
        &rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob
    );
    if (FAILED(hr)) { QFE_REPORT_SYSTEM_ERROR("Failed to serialize root signature", SystemError::Abort); }

    hr = device->CreateRootSignature(
        0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(),
        IID_PPV_ARGS(&rootSignature_)
    );

    // --- 2. Compute PSOの生成 ---
    D3D12_COMPUTE_PIPELINE_STATE_DESC computePsoDesc{};
    computePsoDesc.pRootSignature = rootSignature_.Get();
    computePsoDesc.CS = { csBlob->GetBufferPointer(), csBlob->GetBufferSize() };
    computePsoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

    hr = device->CreateComputePipelineState(&computePsoDesc, IID_PPV_ARGS(&computePipelineState_));
    if (FAILED(hr)) { QFE_REPORT_SYSTEM_ERROR("Failed to create compute pipeline state", SystemError::Abort); }
}
