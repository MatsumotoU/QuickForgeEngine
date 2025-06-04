#include "PipelineStateObject.h"
#include "DirectXCommon.h"
#include "DepthStencil.h"
#include "../Windows/WinApp.h"
#include <cassert>
#include <d3d12.h>

// TODO: 各機能を分離後、初期化と生成を別々の処理にする

void PipelineStateObject::Initialize(DirectXCommon* dxCommon, WinApp* winApp, DepthStencil* depthStencil, const D3D12_PRIMITIVE_TOPOLOGY_TYPE& topologyType, D3D12_FILL_MODE fillMode, const std::string& psFilepath, BlendMode blendMode, bool isParticle) {
	dxCommon_ = dxCommon;
	winApp_ = winApp;
	depthStencil_ = depthStencil;

	// * PSOを作成 * //
	// RootSignatureを作成します
	cBufferManager_.Initialize();
	cBufferManager_.CreateRootParameter(D3D12_ROOT_PARAMETER_TYPE_CBV, D3D12_SHADER_VISIBILITY_PIXEL, 0);

	if (isParticle) {
		cBufferManager_.CreateRootParameter(D3D12_ROOT_PARAMETER_TYPE_SRV, D3D12_SHADER_VISIBILITY_VERTEX, 0);
	} else {
		cBufferManager_.CreateRootParameter(D3D12_ROOT_PARAMETER_TYPE_CBV, D3D12_SHADER_VISIBILITY_VERTEX, 0);
	}
	
	cBufferManager_.CreateRootParameter(D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, D3D12_SHADER_VISIBILITY_PIXEL, 0);
	cBufferManager_.CreateRootParameter(D3D12_ROOT_PARAMETER_TYPE_CBV, D3D12_SHADER_VISIBILITY_PIXEL, 1);

	// DescriptiorRange
	if (isParticle) {
		D3D12_DESCRIPTOR_RANGE descriptorRangeForInstancing[1] = {};
		descriptorRangeForInstancing[0].BaseShaderRegister = 0;
		descriptorRangeForInstancing[0].NumDescriptors = 1;
		descriptorRangeForInstancing[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		descriptorRangeForInstancing[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
		D3D12_ROOT_PARAMETER* particleRootParameters = cBufferManager_.GetRootParameters()->data() + 1;
		particleRootParameters->ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		particleRootParameters->ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
		particleRootParameters->DescriptorTable.pDescriptorRanges = descriptorRangeForInstancing;
		particleRootParameters->DescriptorTable.NumDescriptorRanges = _countof(descriptorRangeForInstancing);

		D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
		descriptorRange[0].BaseShaderRegister = 0;
		descriptorRange[0].NumDescriptors = 1;
		descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
		D3D12_ROOT_PARAMETER* srvRootParameters = cBufferManager_.GetRootParameters()->data() + 2;
		srvRootParameters->DescriptorTable.pDescriptorRanges = descriptorRange;
		srvRootParameters->DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);
	} else {
		D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
		descriptorRange[0].BaseShaderRegister = 0;
		descriptorRange[0].NumDescriptors = 1;
		descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
		D3D12_ROOT_PARAMETER* srvRootParameters = cBufferManager_.GetRootParameters()->data() + 2;
		srvRootParameters->DescriptorTable.pDescriptorRanges = descriptorRange;
		srvRootParameters->DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);
	}
	

	// Samplerの設定
	D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
	staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_POINT_MIP_LINEAR;
	staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;
	staticSamplers[0].ShaderRegister = 0;
	staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	cBufferManager_.GetDescriptionRootSignature()->pStaticSamplers = staticSamplers;
	cBufferManager_.GetDescriptionRootSignature()->NumStaticSamplers = _countof(staticSamplers);

	// シリアライズしてバイナリする
	signatureBlob_ = nullptr;
	errorBlob_ = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(cBufferManager_.GetDescriptionRootSignature(),
		D3D_ROOT_SIGNATURE_VERSION_1, signatureBlob_.GetAddressOf(), errorBlob_.GetAddressOf());
	if (FAILED(hr)) {
		Log(reinterpret_cast<char*>(errorBlob_->GetBufferPointer()));
		assert(false);
	}
	// バイナリをもとに生成
	rootSignature_ = nullptr;
	hr = dxCommon_->GetDevice()->CreateRootSignature(0,
		signatureBlob_.Get()->GetBufferPointer(), signatureBlob_.Get()->GetBufferSize(),
		IID_PPV_ARGS(&rootSignature_));
	assert(SUCCEEDED(hr));

	// InputLayout
	InputLayout inputLayout;
	inputLayout.Initialize();
	inputLayout.CreateInputElementDesc("POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, D3D12_APPEND_ALIGNED_ELEMENT);
	inputLayout.CreateInputElementDesc("TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, D3D12_APPEND_ALIGNED_ELEMENT);
	inputLayout.CreateInputElementDesc("NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, D3D12_APPEND_ALIGNED_ELEMENT);

	// BlendState
	/*D3D12_BLEND_DESC blendDesc{};
	blendDesc.RenderTarget[0].RenderTargetWriteMask =
		D3D12_COLOR_WRITE_ENABLE_ALL;*/
	D3D12_BLEND_DESC blendDesc{};
	blendDesc.AlphaToCoverageEnable = FALSE;
	blendDesc.IndependentBlendEnable = FALSE;
	
	D3D12_RENDER_TARGET_BLEND_DESC& rtbd = blendDesc.RenderTarget[0];
	rtbd.BlendEnable = TRUE; // ブレンドを有効にする
	rtbd.LogicOpEnable = FALSE; // 論理演算は通常 FALSE
	switch (blendMode)
	{
	case BlendMode::kBlendModeNone:
		rtbd.BlendEnable = FALSE; // ブレンドを無効にする
		rtbd.LogicOpEnable = FALSE; // 論理演算は通常 FALSE
		break;
	case BlendMode::kBlendModeNormal:
		rtbd.SrcBlend = D3D12_BLEND_SRC_ALPHA; // ソースのアルファ値を使用
		rtbd.DestBlend = D3D12_BLEND_INV_SRC_ALPHA; // デスティネーションの (1 - ソースアルファ) を使用
		rtbd.BlendOp = D3D12_BLEND_OP_ADD; // 加算
		break;
	case BlendMode::kBlendModeAdd:
		rtbd.SrcBlend = D3D12_BLEND_SRC_ALPHA; // ソースのアルファ値を使用
		rtbd.DestBlend = D3D12_BLEND_ONE;
		rtbd.BlendOp = D3D12_BLEND_OP_ADD; // 加算
		break;
	case BlendMode::kBlendModeSubtract:
		rtbd.SrcBlend = D3D12_BLEND_SRC_ALPHA; // ソースのアルファ値を使用
		rtbd.DestBlend = D3D12_BLEND_ONE;
		rtbd.BlendOp = D3D12_BLEND_OP_REV_SUBTRACT; // 減算
		break;
	case BlendMode::kBlendModeMultily:
		rtbd.SrcBlend = D3D12_BLEND_ZERO; 
		rtbd.DestBlend = D3D12_BLEND_SRC_COLOR;
		rtbd.BlendOp = D3D12_BLEND_OP_ADD; // 乗算
		break;
	case BlendMode::kBlendModeScreen:
		rtbd.SrcBlend = D3D12_BLEND_INV_DEST_COLOR;
		rtbd.DestBlend = D3D12_BLEND_ONE;
		rtbd.BlendOp = D3D12_BLEND_OP_ADD; // 乗算
		break;
	case BlendMode::kCountOfBlendMode:
		assert(false && "useBlendMode kCountOfBlendMode");
		break;
	default:
		assert(false && "useBlendMode UnknownBlendMode");
		break;
	}

	// アルファ ブレンドの設定 (通常はソースのアルファ値をそのまま使用)
	rtbd.SrcBlendAlpha = D3D12_BLEND_ONE; // ソースのアルファ値をそのまま使用
	rtbd.DestBlendAlpha = D3D12_BLEND_ZERO; // デスティネーションのアルファ値に 0 を掛ける
	rtbd.BlendOpAlpha = D3D12_BLEND_OP_ADD; // 加算

	rtbd.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	// RasterizerState
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	// 裏面（時計回り）を表示しない
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	// 塗りつぶし
	rasterizerDesc.FillMode = fillMode;

	// シェーダーをコンパイルする
	shaderCompiler_.InitializeDXC();
	IDxcBlob* vertexShaderBlob = nullptr;
	IDxcBlob* pixelShaderBlob = nullptr;
	if (isParticle) {
		vertexShaderBlob = shaderCompiler_.CompileShader(L"Particle.VS.hlsl", L"vs_6_0");
		assert(vertexShaderBlob != nullptr);
		pixelShaderBlob = shaderCompiler_.CompileShader(ConvertString(psFilepath), L"ps_6_0");
		assert(pixelShaderBlob != nullptr);
	} else {
		vertexShaderBlob = shaderCompiler_.CompileShader(L"Object3d.VS.hlsl", L"vs_6_0");
		assert(vertexShaderBlob != nullptr);
		pixelShaderBlob = shaderCompiler_.CompileShader(ConvertString(psFilepath), L"ps_6_0");
		assert(pixelShaderBlob != nullptr);
	}
	

	// DepthStencilState
	//depthStencil_->Initialize(winApp_,dxCommon_);

	// PSOを生成
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
	graphicsPipelineStateDesc.DepthStencilState = *depthStencil_->GetDepthStencilDesc();
	graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	graphicsPipelineStateDesc.pRootSignature = rootSignature_.Get();
	graphicsPipelineStateDesc.InputLayout = *inputLayout.GetInputLayoutDesc();
	graphicsPipelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(),
	vertexShaderBlob->GetBufferSize() };
	graphicsPipelineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(),
	pixelShaderBlob->GetBufferSize() };
	graphicsPipelineStateDesc.BlendState = blendDesc;
	graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;
	// 書き込むRTVの情報
	graphicsPipelineStateDesc.NumRenderTargets = 1;
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	// 利用するトロポジ
	graphicsPipelineStateDesc.PrimitiveTopologyType = topologyType;
	// どのように画面に色を打ち込むかの設定
	graphicsPipelineStateDesc.SampleDesc.Count = 1;
	graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	// 実際に生成
	graphicsPipelineState_ = nullptr;
	hr = dxCommon_->GetDevice()->CreateGraphicsPipelineState(&graphicsPipelineStateDesc,
		IID_PPV_ARGS(graphicsPipelineState_.GetAddressOf()));
	assert(SUCCEEDED(hr));
}

ID3D12PipelineState* PipelineStateObject::GetPipelineState() {
	return graphicsPipelineState_.Get();
}

ID3D12RootSignature* PipelineStateObject::GetRootSignature() {
	return rootSignature_.Get();
}

DepthStencil* PipelineStateObject::GetDepthStencil() {
	return depthStencil_;
}
