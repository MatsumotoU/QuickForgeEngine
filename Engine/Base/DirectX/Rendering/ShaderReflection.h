#pragma once
#include <d3d12shader.h>
#include <d3dcompiler.h>
#include <wrl.h>
#include <vector>
#include <string>
#include <d3d12.h>

class ShaderReflection {
public:
    ShaderReflection();
    ~ShaderReflection();

    // シェーダーバイトコードからリフレクション情報を取得
    bool Reflect(const void* shaderBytecode, size_t bytecodeLength);

    // ルートシグネチャ自動生成
    bool GenerateRootSignatureDesc(D3D12_ROOT_SIGNATURE_DESC& outDesc);

    // バインディング情報取得
    struct BindingInfo {
        std::string name;
        D3D_SHADER_INPUT_TYPE type;
        UINT bindPoint;
        UINT bindCount;
        UINT space;
    };
    std::vector<BindingInfo> GetBindingInfos() const;

    // 解放
    void Release();

private:
    Microsoft::WRL::ComPtr<ID3D12ShaderReflection> shaderReflection_;
    D3D12_SHADER_DESC shaderDesc_{};
    std::vector<BindingInfo> bindingInfos_;
};