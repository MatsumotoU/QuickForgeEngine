#include "RasterizerTemplate.h"
#include "EngineDefines.h"

using namespace QFE::GRAPHIC::INTERNAL;

void RasterizerTemplate::Initialize() {
	// Default(背面をカリングして、塗りつぶす)
    D3D12_RASTERIZER_DESC defaultDesc{};
    defaultDesc.FillMode = D3D12_FILL_MODE_SOLID;
    defaultDesc.CullMode = D3D12_CULL_MODE_BACK;
    defaultDesc.FrontCounterClockwise = FALSE;
    defaultDesc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
    defaultDesc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
    defaultDesc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
    defaultDesc.DepthClipEnable = TRUE;
    defaultDesc.MultisampleEnable = FALSE;
    defaultDesc.AntialiasedLineEnable = FALSE;
    defaultDesc.ForcedSampleCount = 0;
    defaultDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
    rasterizerDescs_[RasterizerType::Default] = defaultDesc;
	// Wireframe(ワイヤーフレームで描画)
    D3D12_RASTERIZER_DESC wireframeDesc = defaultDesc;
    wireframeDesc.FillMode = D3D12_FILL_MODE_WIREFRAME;
    rasterizerDescs_[RasterizerType::Wireframe] = wireframeDesc;
	// CullNone(背面をカリングしないで、塗りつぶす)
    D3D12_RASTERIZER_DESC cullNoneDesc = defaultDesc;
    cullNoneDesc.CullMode = D3D12_CULL_MODE_NONE;
	rasterizerDescs_[RasterizerType::CullNone] = cullNoneDesc;

	QFE_LOG("RasterizerTemplate: Initialized rasterizer descriptions for Default, Wireframe, and CullNone.");
}

const D3D12_RASTERIZER_DESC& RasterizerTemplate::GetRasterizerDesc(RasterizerType type) const {
    auto it = rasterizerDescs_.find(type);
    if (it != rasterizerDescs_.end()) {
        return it->second;
    } else {
		// 指定されたRasterizerTypeが見つからない場合は、Defaultの設定を返す
        QFE_LOG("RasterizerTemplate: RasterizerType not found. Returning Default rasterizer description.");
        return rasterizerDescs_.at(RasterizerType::Default);
	}
}

std::unordered_map<RasterizerType, D3D12_RASTERIZER_DESC> QFE::GRAPHIC::INTERNAL::RasterizerTemplate::GetRasterizerDescMap() const {
	return rasterizerDescs_;
}

uint32_t QFE::GRAPHIC::INTERNAL::RasterizerTemplate::GetRasterizerTypeCount() const {
	return static_cast<uint32_t>(RasterizerType::kCount);
}
