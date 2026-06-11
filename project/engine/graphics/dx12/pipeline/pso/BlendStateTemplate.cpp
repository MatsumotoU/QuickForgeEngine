#include "BlendStateTemplate.h"

using namespace QFE::GRAPHIC::INTERNAL;

void BlendStateTemplate::Initialize() {
	D3D12_BLEND_DESC blendDesc{};
	blendDesc.AlphaToCoverageEnable = FALSE;
	blendDesc.IndependentBlendEnable = FALSE;


	D3D12_RENDER_TARGET_BLEND_DESC& rtbd = blendDesc.RenderTarget[0];
	rtbd.BlendEnable = TRUE; // ブレンドを有効にする
	rtbd.LogicOpEnable = FALSE; // 論理演算は通常FALSE

	// アルファブレンドの設定 (通常はソースのアルファ値をそのまま使用)
	rtbd.SrcBlendAlpha = D3D12_BLEND_ONE; // ソースのアルファ値をそのまま使用
	rtbd.DestBlendAlpha = D3D12_BLEND_ZERO; // デスティネーションのアルファ値に 0 を掛ける
	rtbd.BlendOpAlpha = D3D12_BLEND_OP_ADD; // 加算
	rtbd.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	// ブレンドモードごとの設定
	// ブレンドなし
	rtbd.BlendEnable = FALSE; // ブレンドを無効にする
	rtbd.LogicOpEnable = FALSE; // 論理演算は通常FALSE
	blendDescMap_[BlendMode::kBlendModeNone] = blendDesc;
	// 通常
	rtbd.SrcBlend = D3D12_BLEND_SRC_ALPHA; // ソースのアルファ値を使用
	rtbd.DestBlend = D3D12_BLEND_INV_SRC_ALPHA; // デスティネーションの (1 - ソースアルファ) を使用
	rtbd.BlendOp = D3D12_BLEND_OP_ADD; // 加算
	blendDescMap_[BlendMode::kBlendModeNormal] = blendDesc;
	// 加算
	rtbd.SrcBlend = D3D12_BLEND_SRC_ALPHA; // ソースのアルファ値を使用
	rtbd.DestBlend = D3D12_BLEND_ONE;
	rtbd.BlendOp = D3D12_BLEND_OP_ADD; // 加算
	blendDescMap_[BlendMode::kBlendModeAdd] = blendDesc;
	// 減算
	rtbd.SrcBlend = D3D12_BLEND_SRC_ALPHA; // ソースのアルファ値を使用
	rtbd.DestBlend = D3D12_BLEND_ONE;
	rtbd.BlendOp = D3D12_BLEND_OP_REV_SUBTRACT; // 減算
	blendDescMap_[BlendMode::kBlendModeSubtract] = blendDesc;
	// 乗算
	rtbd.SrcBlend = D3D12_BLEND_ZERO;
	rtbd.DestBlend = D3D12_BLEND_SRC_COLOR;
	rtbd.BlendOp = D3D12_BLEND_OP_ADD; // 乗算 (OpはAddだがSrcとDestの組み合わせで乗算となる)
	blendDescMap_[BlendMode::kBlendModeMultiply] = blendDesc;
	// スクリーン
	rtbd.SrcBlend = D3D12_BLEND_INV_DEST_COLOR;
	rtbd.DestBlend = D3D12_BLEND_ONE;
	rtbd.BlendOp = D3D12_BLEND_OP_ADD; // スクリーン
	blendDescMap_[BlendMode::kBlendModeScreen] = blendDesc;
}

D3D12_BLEND_DESC BlendStateTemplate::GetBlendDesc(BlendMode mode) const {
	if(blendDescMap_.find(mode) != blendDescMap_.end()) {
		return blendDescMap_.at(mode);
	}
	return blendDescMap_.at(BlendMode::kBlendModeNone);
}

std::unordered_map<BlendMode, D3D12_BLEND_DESC> QFE::GRAPHIC::INTERNAL::BlendStateTemplate::GetBlendDescMap() const {
	return blendDescMap_;
}

uint32_t QFE::GRAPHIC::INTERNAL::BlendStateTemplate::GetBlendModeCount() const {
	return static_cast<uint32_t>(BlendMode::kCountOfBlendMode);
}
