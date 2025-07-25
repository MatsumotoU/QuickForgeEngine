#include "ShaderReflection.h"

ShaderReflection::ShaderReflection() = default;
ShaderReflection::~ShaderReflection() { Release(); }

bool ShaderReflection::Reflect(const void* shaderBytecode, size_t bytecodeLength) {
    Release();
    HRESULT hr = D3DReflect(shaderBytecode, bytecodeLength, IID_PPV_ARGS(&shaderReflection_));
    if (FAILED(hr) || !shaderReflection_) return false;

    shaderReflection_->GetDesc(&shaderDesc_);
    bindingInfos_.clear();
    for (UINT i = 0; i < shaderDesc_.BoundResources; ++i) {
        D3D12_SHADER_INPUT_BIND_DESC bindDesc{};
        shaderReflection_->GetResourceBindingDesc(i, &bindDesc);
        bindingInfos_.push_back({
            bindDesc.Name,
            bindDesc.Type,
            bindDesc.BindPoint,
            bindDesc.BindCount,
            bindDesc.Space
            });
    }
    return true;
}

std::vector<ShaderReflection::BindingInfo> ShaderReflection::GetBindingInfos() const {
    return bindingInfos_;
}

bool ShaderReflection::GenerateRootSignatureDesc(D3D12_ROOT_SIGNATURE_DESC& outDesc) {
    if (!shaderReflection_) return false;

    std::vector<D3D12_ROOT_PARAMETER> rootParams;
    for (const auto& info : bindingInfos_) {
        if (info.type == D3D_SIT_CBUFFER) {
            D3D12_ROOT_PARAMETER param{};
            param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
            param.Descriptor.ShaderRegister = info.bindPoint;
            param.Descriptor.RegisterSpace = info.space;
            param.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
            rootParams.push_back(param);
        }
    }
    outDesc.NumParameters = static_cast<UINT>(rootParams.size());
    outDesc.pParameters = rootParams.empty() ? nullptr : new D3D12_ROOT_PARAMETER[rootParams.size()];
    if (outDesc.pParameters) {
        std::copy(rootParams.begin(), rootParams.end(), const_cast<D3D12_ROOT_PARAMETER*>(outDesc.pParameters));
    }
    outDesc.NumStaticSamplers = 0;
    outDesc.pStaticSamplers = nullptr;
    outDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
    return true;
}

void ShaderReflection::Release() {
    shaderReflection_.Reset();
    bindingInfos_.clear();
}
