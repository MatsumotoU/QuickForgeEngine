#include "DepthStencilDescTemplate.h"
#include "EngineDefines.h"

using namespace QFE::GRAPHIC::INTERNAL;

void DepthStencilDescTemplate::Initialize() {
	// デフォルトの深度ステンシルステートを作成
    D3D12_DEPTH_STENCIL_DESC defaultDesc = {};
    defaultDesc.DepthEnable = TRUE;
    defaultDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    defaultDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
    defaultDesc.StencilEnable = FALSE;
	// 半透明用の深度ステンシルステートを作成
    D3D12_DEPTH_STENCIL_DESC translucentDesc = {};
    translucentDesc.DepthEnable = TRUE;
    translucentDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
    translucentDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
    translucentDesc.StencilEnable = FALSE;
	// 深度テストも書き込みも行わないステンシルステートを作成
    D3D12_DEPTH_STENCIL_DESC noneDesc = {};
    noneDesc.DepthEnable = FALSE;
    noneDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
    noneDesc.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;
    noneDesc.StencilEnable = FALSE;

	// 各種ステンシルステートをマップに保存
    descs[DepthStencilDescType::Default] = defaultDesc;
    descs[DepthStencilDescType::Translucent] = translucentDesc;
    descs[DepthStencilDescType::None] = noneDesc;
}

const D3D12_DEPTH_STENCIL_DESC& DepthStencilDescTemplate::GetDesc(DepthStencilDescType type) const {
    if(decltype(auto) it = descs.find(type); it == descs.end()) {
		QFE_LOG("Error: DepthStencilDescType not found in descs map.");
		return descs.at(DepthStencilDescType::Default);
	}
    return descs.at(type);
}
