#include "StaticSamplerTemplate.h"

#include "EngineDefines.h"

using namespace QFE::GRAPHIC::INTERNAL;

void StaticSamplerTemplate::Initialize() {
	samplerDescs_.clear();
	samplerDescs_.resize(6); // 6種類のサンプラーを用意

    // S0. PointWrap (ドット絵・リピート)
	D3D12_STATIC_SAMPLER_DESC pointWrapSamplerDesc{};
    pointWrapSamplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
    pointWrapSamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    pointWrapSamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    pointWrapSamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    pointWrapSamplerDesc.ShaderRegister = 0; // register(s0)
    pointWrapSamplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    // S1. PointClamp (ドット絵・端の色固定)
    D3D12_STATIC_SAMPLER_DESC pointClampSamplerDesc = pointWrapSamplerDesc;
    pointClampSamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    pointClampSamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    pointClampSamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    pointClampSamplerDesc.ShaderRegister = 1; // register(s1)

    // S2. LinearWrap (標準的な滑らかさ・リピート)
    D3D12_STATIC_SAMPLER_DESC linearWrapSamplerDesc = pointWrapSamplerDesc;
    linearWrapSamplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    linearWrapSamplerDesc.ShaderRegister = 2; // register(s2)

    // S3. LinearClamp (UIや画面全体処理・端の色固定)
    D3D12_STATIC_SAMPLER_DESC linearClampSamplerDesc = pointClampSamplerDesc;
    linearClampSamplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    linearClampSamplerDesc.ShaderRegister = 3; // register(s3)

    // S4. AnisotropicWrap (地面や壁のボケ防止・異方性フィルタ)
    D3D12_STATIC_SAMPLER_DESC anisotropicWrapSamplerDesc = pointWrapSamplerDesc;
    anisotropicWrapSamplerDesc.Filter = D3D12_FILTER_ANISOTROPIC;
    anisotropicWrapSamplerDesc.MaxAnisotropy = 16;
    anisotropicWrapSamplerDesc.ShaderRegister = 4; // register(s4)

    // S5. ShadowMap (影の境界線をスムーズにする比較機能付き)
    D3D12_STATIC_SAMPLER_DESC shadowSamplerDesc = {};
    shadowSamplerDesc.Filter = D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
    shadowSamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER; // 影の外側は黒
    shadowSamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    shadowSamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    shadowSamplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE; // 比較用に白
    shadowSamplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;     // 影の深度比較用
    shadowSamplerDesc.ShaderRegister = 5; // register(s5)
    shadowSamplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	// サンプラー設定を配列に格納
	samplerDescs_[0] = pointWrapSamplerDesc;
	samplerDescs_[1] = pointClampSamplerDesc;
	samplerDescs_[2] = linearWrapSamplerDesc;
	samplerDescs_[3] = linearClampSamplerDesc;
	samplerDescs_[4] = anisotropicWrapSamplerDesc;
	samplerDescs_[5] = shadowSamplerDesc;

	isInitialized_ = true;
}

const D3D12_STATIC_SAMPLER_DESC* QFE::GRAPHIC::INTERNAL::StaticSamplerTemplate::GetSamplerDescs() const {
	CheckInitialized();
	return samplerDescs_.data();
}

UINT QFE::GRAPHIC::INTERNAL::StaticSamplerTemplate::GetSamplerCount() const {
	CheckInitialized();
	return static_cast<UINT>(samplerDescs_.size());
}

void QFE::GRAPHIC::INTERNAL::StaticSamplerTemplate::CheckInitialized() const {
    if (!isInitialized_) {
        QFE_REPORT_SYSTEM_ERROR(std::string("StaticSamplerTemplate: Sampler descriptions are not initialized. Call Initialize() before using."), SystemError::Abort);
	}
}
