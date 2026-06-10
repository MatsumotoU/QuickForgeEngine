#include "BlendStates.h"

using namespace QFE::GRAPHIC::INTERNAL;

void BlendStates::Initialize() {
	// ブレンドステートの設定を生成
	D3D12_BLEND_DESC noneDesc{};
	noneDesc.AlphaToCoverageEnable = FALSE;
	noneDesc.IndependentBlendEnable = FALSE;
	noneDesc.RenderTarget[0].BlendEnable = FALSE;
	// 通常ブレンドの設定
	D3D12_BLEND_DESC normalDesc{};
	normalDesc.AlphaToCoverageEnable = FALSE;
	normalDesc.IndependentBlendEnable = FALSE;
	normalDesc.RenderTarget[0].BlendEnable = TRUE;
	normalDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	normalDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	normalDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	// 加算ブレンドの設定
	D3D12_BLEND_DESC addDesc{};
	addDesc.AlphaToCoverageEnable = FALSE;
	addDesc.IndependentBlendEnable = FALSE;
	addDesc.RenderTarget[0].BlendEnable = TRUE;
	addDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	addDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
	addDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	// 減算ブレンドの設定
	D3D12_BLEND_DESC subtractDesc{};
	subtractDesc.AlphaToCoverageEnable = FALSE;
	subtractDesc.IndependentBlendEnable = FALSE;
	subtractDesc.RenderTarget[0].BlendEnable = TRUE;
	subtractDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	subtractDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
	subtractDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_REV_SUBTRACT;
	// 乗算ブレンドの設定
	D3D12_BLEND_DESC multiplyDesc{};
	multiplyDesc.AlphaToCoverageEnable = FALSE;
	multiplyDesc.IndependentBlendEnable = FALSE;
	multiplyDesc.RenderTarget[0].BlendEnable = TRUE;
	multiplyDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_ZERO;
	multiplyDesc.RenderTarget[0].DestBlend = D3D12_BLEND_SRC_COLOR;
	multiplyDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	// スクリーンブレンドの設定
	D3D12_BLEND_DESC screenDesc{};
	screenDesc.AlphaToCoverageEnable = FALSE;
	screenDesc.IndependentBlendEnable = FALSE;
	screenDesc.RenderTarget[0].BlendEnable = TRUE;
	screenDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_INV_DEST_COLOR;
	screenDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
	screenDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;

	// ブレンドステートの設定をマップに格納
	blendDescs_[BlendMode::kBlendModeNone] = noneDesc;
	blendDescs_[BlendMode::kBlendModeNormal] = normalDesc;
	blendDescs_[BlendMode::kBlendModeAdd] = addDesc;
	blendDescs_[BlendMode::kBlendModeSubtract] = subtractDesc;
	blendDescs_[BlendMode::kBlendModeMultiply] = multiplyDesc;
	blendDescs_[BlendMode::kBlendModeScreen] = screenDesc;
}

const D3D12_BLEND_DESC& BlendStates::GetBlendDesc(BlendMode mode) const {
	auto it = blendDescs_.find(mode);
	if (it != blendDescs_.end()) {
		return it->second;
	}
	// デフォルトのブレンドステートを返す
	static D3D12_BLEND_DESC defaultBlendDesc = {};
	return defaultBlendDesc;
}