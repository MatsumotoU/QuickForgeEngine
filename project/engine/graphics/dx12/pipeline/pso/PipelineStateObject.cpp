#include "PipelineStateObject.h"
#include <cassert>
#include <d3d12.h>
#include <stdexcept>

#include "EngineDefines.h"

using namespace QFE::GRAPHIC;

void QFE::GRAPHIC::PipelineStateObject::CreatePipelineStateObject(
	const PipelineStateObjectElement& element, ID3D12Device* device) {
	// 既に生成されているならスキップ
	if (isCreatedPipelineStateObject_) {
		QFE_LOG(std::string("PipelineStateObject: PipelineStateObject is already created. Skipping creation."));
		return;
	}
	// 引数のチェック
	if (element.rootParameter == nullptr) {
		QFE_REPORT_SYSTEM_ERROR(std::string("RootParameter is null"), SystemError::Abort);
	}
	if (element.inputLayoutDesc == nullptr) {
		QFE_REPORT_SYSTEM_ERROR(std::string("InputLayoutDesc is null"), SystemError::Abort);
	}
	if (element.psBlob == nullptr) {
		QFE_REPORT_SYSTEM_ERROR(std::string("PixelShaderBlob is null"), SystemError::Abort);
	}
	if (element.vsBlob == nullptr) {
		QFE_REPORT_SYSTEM_ERROR(std::string("VertexShaderBlob is null"), SystemError::Abort);
	}
	if (device == nullptr) {
		QFE_REPORT_SYSTEM_ERROR(std::string("Device is null"), SystemError::Abort);
	}

	shaderPairHandle_ = element.shaderPairHandle;

	HRESULT hr;
	// * RootSignature * //
	// シェーダーとリソースをどのように繋ぐかを定義している
	signatureBlob_ = nullptr;
	errorBlob_ = nullptr;
	// RootSignatureをシリアライズ
	hr = D3D12SerializeRootSignature(element.rootParameter,
		D3D_ROOT_SIGNATURE_VERSION_1, signatureBlob_.GetAddressOf(), errorBlob_.GetAddressOf());
	if (FAILED(hr)) {
		QFE_REPORT_SYSTEM_ERROR(reinterpret_cast<char*>(errorBlob_->GetBufferPointer()), SystemError::Abort);
		assert(false);
	}
	// RootSignatureを生成
	rootSignature_ = nullptr;
	hr = device->CreateRootSignature(0,
		signatureBlob_.Get()->GetBufferPointer(), signatureBlob_.Get()->GetBufferSize(),
		IID_PPV_ARGS(&rootSignature_));
	assert(SUCCEEDED(hr));
	if (FAILED(hr)) {
		QFE_REPORT_SYSTEM_ERROR(std::string("PipelineStateObject: Failed to create root signature"), SystemError::Abort);
	}

	// PSOを生成
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
	graphicsPipelineStateDesc.DepthStencilState = element.depthStencilDesc;
	if (element.depthStencilDesc.DepthEnable) {
		graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	} else {
		graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_UNKNOWN;
	}
	// RootSignatureを設定
	graphicsPipelineStateDesc.pRootSignature = rootSignature_.Get();
	graphicsPipelineStateDesc.InputLayout = *element.inputLayoutDesc;
	graphicsPipelineStateDesc.VS = { element.vsBlob->GetBufferPointer(),
	element.vsBlob->GetBufferSize() };
	graphicsPipelineStateDesc.PS = { element.psBlob->GetBufferPointer(),
	element.psBlob->GetBufferSize() };
	graphicsPipelineStateDesc.BlendState = element.blendDesc;
	graphicsPipelineStateDesc.RasterizerState = element.rasterizerDesc;
	// 書き込むRTVの情報
	graphicsPipelineStateDesc.NumRenderTargets = element.numRenderTarget;
	for(UINT i = 0; i < element.numRenderTarget; ++i) {
		graphicsPipelineStateDesc.RTVFormats[i] = element.renderTargetFormat;
	}
	// 利用するトポロジ
	graphicsPipelineStateDesc.PrimitiveTopologyType = element.topologyType;
	// マルチサンプルの設定
	graphicsPipelineStateDesc.SampleDesc.Count = 1;
	graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	// 実際に生成
	graphicsPipelineState_ = nullptr;
	hr = device->CreateGraphicsPipelineState(&graphicsPipelineStateDesc,
		IID_PPV_ARGS(graphicsPipelineState_.GetAddressOf()));
	assert(SUCCEEDED(hr));

	isCreatedPipelineStateObject_ = true;
}

ID3D12PipelineState* PipelineStateObject::GetPipelineState() {
	// そもそも生成されていない場合はエラー
	if (!isCreatedPipelineStateObject_) {
		QFE_REPORT_SYSTEM_ERROR(std::string("PipelineStateObject: PipelineStateObject is not created."), SystemError::Abort);
	}

	// PSOが作成されていない場合はエラー
	if(graphicsPipelineState_ == nullptr) {
		QFE_REPORT_SYSTEM_ERROR(std::string("PipelineStateObject: PipelineState is not created."), SystemError::Abort);
	}

	return graphicsPipelineState_.Get();
}

ID3D12RootSignature* PipelineStateObject::GetRootSignature() {
	// そもそも生成されていない場合はエラー
	if (!isCreatedPipelineStateObject_) {
		QFE_REPORT_SYSTEM_ERROR(std::string("PipelineStateObject: PipelineStateObject is not created."), SystemError::Abort);
	}

	// RootSignatureが作成されていない場合はエラー
	if(rootSignature_ == nullptr) {
		QFE_REPORT_SYSTEM_ERROR(std::string("PipelineStateObject: RootSignature is not created."), SystemError::Abort);
	}

	return rootSignature_.Get();
}

uint32_t QFE::GRAPHIC::PipelineStateObject::GetShaderPairHandle() const {
	return shaderPairHandle_;
}
