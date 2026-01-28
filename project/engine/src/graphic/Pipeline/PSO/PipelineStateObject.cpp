#include "engine/include/graphic/Pipeline/PSO/PipelineStateObject.h"
#include <cassert>
#include <d3d12.h>
using namespace QFE;
namespace {
	const std::string kVSFilePath = "engine/resources/shaders/vs/";
	const std::string kPSFilePath = "engine/resources/shaders/ps/";
}

void PipelineStateObject::Initialize(ShaderCompiler* shaderCompiler, ID3D12Device* device) {
	shaderCompiler_ = shaderCompiler;
	dxDevice_ = device;
	assert(shaderCompiler_ != nullptr);
	assert(dxDevice_ != nullptr);
}

void PipelineStateObject::CreatePipelineStateObject(
	RootParameter rootParameter, D3D12_DEPTH_STENCIL_DESC depthStencilDesc, InputLayout inputLayout, const D3D12_PRIMITIVE_TOPOLOGY_TYPE& topologyType,
	D3D12_FILL_MODE fillMode, const std::string& psFilepath, const std::string& vsFilepath, BlendMode blendMode, bool isDrawBack) {
	HRESULT hr{};

	// サンプラー設定
	D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
	staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_POINT_MIP_LINEAR;
	staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;
	staticSamplers[0].ShaderRegister = 0;
	staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameter.GetDescriptionRootSignature()->pStaticSamplers = staticSamplers;
	rootParameter.GetDescriptionRootSignature()->NumStaticSamplers = _countof(staticSamplers);

	// * RootSignature * //
	// シェーダーとリソースをどのように繋ぐかを定義している
	signatureBlob_ = nullptr;
	errorBlob_ = nullptr;
	hr = D3D12SerializeRootSignature(rootParameter.GetDescriptionRootSignature(),
		D3D_ROOT_SIGNATURE_VERSION_1, signatureBlob_.GetAddressOf(), errorBlob_.GetAddressOf());
	if (FAILED(hr)) {
		Log(reinterpret_cast<char*>(errorBlob_->GetBufferPointer()));
		assert(false);
	}
	// 
	rootSignature_ = nullptr;
	hr = dxDevice_->CreateRootSignature(0,
		signatureBlob_.Get()->GetBufferPointer(), signatureBlob_.Get()->GetBufferSize(),
		IID_PPV_ARGS(&rootSignature_));
	assert(SUCCEEDED(hr));

	// * BlendState * //
	// 
	D3D12_BLEND_DESC blendDesc{};
	blendDesc.AlphaToCoverageEnable = FALSE;
	blendDesc.IndependentBlendEnable = FALSE;

	D3D12_RENDER_TARGET_BLEND_DESC& rtbd = blendDesc.RenderTarget[0];
	rtbd.BlendEnable = TRUE; // 繝悶Ξ繝ｳ繝峨ｒ譛牙柑縺ｫ縺吶ｋ
	rtbd.LogicOpEnable = FALSE; // 隲也炊貍皮ｮ励・騾壼ｸｸ FALSE
	switch (blendMode)
	{
	case BlendMode::kBlendModeNone:
		rtbd.BlendEnable = FALSE; // 繝悶Ξ繝ｳ繝峨ｒ辟｡蜉ｹ縺ｫ縺吶ｋ
		rtbd.LogicOpEnable = FALSE; // 隲也炊貍皮ｮ励・騾壼ｸｸ FALSE
		break;
	case BlendMode::kBlendModeNormal:
		rtbd.SrcBlend = D3D12_BLEND_SRC_ALPHA; // 繧ｽ繝ｼ繧ｹ縺ｮ繧｢繝ｫ繝輔ぃ蛟､繧剃ｽｿ逕ｨ
		rtbd.DestBlend = D3D12_BLEND_INV_SRC_ALPHA; // 繝・せ繝・ぅ繝阪・繧ｷ繝ｧ繝ｳ縺ｮ (1 - 繧ｽ繝ｼ繧ｹ繧｢繝ｫ繝輔ぃ) 繧剃ｽｿ逕ｨ
		rtbd.BlendOp = D3D12_BLEND_OP_ADD; // 蜉邂・
		break;
	case BlendMode::kBlendModeAdd:
		rtbd.SrcBlend = D3D12_BLEND_SRC_ALPHA; // 繧ｽ繝ｼ繧ｹ縺ｮ繧｢繝ｫ繝輔ぃ蛟､繧剃ｽｿ逕ｨ
		rtbd.DestBlend = D3D12_BLEND_ONE;
		rtbd.BlendOp = D3D12_BLEND_OP_ADD; // 蜉邂・
		break;
	case BlendMode::kBlendModeSubtract:
		rtbd.SrcBlend = D3D12_BLEND_SRC_ALPHA; // 繧ｽ繝ｼ繧ｹ縺ｮ繧｢繝ｫ繝輔ぃ蛟､繧剃ｽｿ逕ｨ
		rtbd.DestBlend = D3D12_BLEND_ONE;
		rtbd.BlendOp = D3D12_BLEND_OP_REV_SUBTRACT; // 貂帷ｮ・
		break;
	case BlendMode::kBlendModeMultily:
		rtbd.SrcBlend = D3D12_BLEND_ZERO;
		rtbd.DestBlend = D3D12_BLEND_SRC_COLOR;
		rtbd.BlendOp = D3D12_BLEND_OP_ADD; // 荵礼ｮ・
		break;
	case BlendMode::kBlendModeScreen:
		rtbd.SrcBlend = D3D12_BLEND_INV_DEST_COLOR;
		rtbd.DestBlend = D3D12_BLEND_ONE;
		rtbd.BlendOp = D3D12_BLEND_OP_ADD; // 荵礼ｮ・
		break;
	case BlendMode::kCountOfBlendMode:
		assert(false && "useBlendMode kCountOfBlendMode");
		break;
	default:
		assert(false && "useBlendMode UnknownBlendMode");
		break;
	}

	// 繧｢繝ｫ繝輔ぃ 繝悶Ξ繝ｳ繝峨・險ｭ螳・(騾壼ｸｸ縺ｯ繧ｽ繝ｼ繧ｹ縺ｮ繧｢繝ｫ繝輔ぃ蛟､繧偵◎縺ｮ縺ｾ縺ｾ菴ｿ逕ｨ)
	rtbd.SrcBlendAlpha = D3D12_BLEND_ONE; // 繧ｽ繝ｼ繧ｹ縺ｮ繧｢繝ｫ繝輔ぃ蛟､繧偵◎縺ｮ縺ｾ縺ｾ菴ｿ逕ｨ
	rtbd.DestBlendAlpha = D3D12_BLEND_ZERO; // 繝・せ繝・ぅ繝阪・繧ｷ繝ｧ繝ｳ縺ｮ繧｢繝ｫ繝輔ぃ蛟､縺ｫ 0 繧呈寺縺代ｋ
	rtbd.BlendOpAlpha = D3D12_BLEND_OP_ADD; // 蜉邂・

	rtbd.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	// RasterizerState
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	if (isDrawBack) {
		// 陬城擇・域凾險亥屓繧奇ｼ峨ｒ陦ｨ遉ｺ縺吶ｋ
		rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;

	} else {
		// 陬城擇・域凾險亥屓繧奇ｼ峨ｒ陦ｨ遉ｺ縺励↑縺・
		rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	}
	// 蝪励ｊ縺､縺ｶ縺・
	rasterizerDesc.FillMode = fillMode;

	// 繧ｷ繧ｧ繝ｼ繝繝ｼ繧偵さ繝ｳ繝代う繝ｫ縺吶ｋ
	IDxcBlob* vertexShaderBlob = nullptr;
	IDxcBlob* pixelShaderBlob = nullptr;
	vertexShaderBlob = shaderCompiler_->CompileShader(ConvertString(kVSFilePath + vsFilepath), L"vs_6_0");
	assert(vertexShaderBlob != nullptr);
	pixelShaderBlob = shaderCompiler_->CompileShader(ConvertString(kPSFilePath + psFilepath), L"ps_6_0");
	assert(pixelShaderBlob != nullptr);

	// PSO繧堤函謌・
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
	graphicsPipelineStateDesc.DepthStencilState = depthStencilDesc;
	if (depthStencilDesc.DepthEnable) {
		graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	} else {
		graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_UNKNOWN;
	}

	graphicsPipelineStateDesc.pRootSignature = rootSignature_.Get();
	graphicsPipelineStateDesc.InputLayout = *inputLayout.GetInputLayoutDesc();
	graphicsPipelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(),
	vertexShaderBlob->GetBufferSize() };
	graphicsPipelineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(),
	pixelShaderBlob->GetBufferSize() };
	graphicsPipelineStateDesc.BlendState = blendDesc;
	graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;
	// 譖ｸ縺崎ｾｼ繧RTV縺ｮ諠・ｱ
	graphicsPipelineStateDesc.NumRenderTargets = 1;
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	// 蛻ｩ逕ｨ縺吶ｋ繝医Ο繝昴ず
	graphicsPipelineStateDesc.PrimitiveTopologyType = topologyType;
	// 縺ｩ縺ｮ繧医≧縺ｫ逕ｻ髱｢縺ｫ濶ｲ繧呈遠縺｡霎ｼ繧縺九・險ｭ螳・
	graphicsPipelineStateDesc.SampleDesc.Count = 1;
	graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	// 螳滄圀縺ｫ逕滓・
	graphicsPipelineState_ = nullptr;
	hr = dxDevice_->CreateGraphicsPipelineState(&graphicsPipelineStateDesc,
		IID_PPV_ARGS(graphicsPipelineState_.GetAddressOf()));
	assert(SUCCEEDED(hr));
}

ID3D12PipelineState* PipelineStateObject::GetPipelineState() {
	return graphicsPipelineState_.Get();
}

ID3D12RootSignature* PipelineStateObject::GetRootSignature() {
	return rootSignature_.Get();
}
