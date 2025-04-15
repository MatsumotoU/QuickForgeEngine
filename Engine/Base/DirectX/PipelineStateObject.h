#pragma once
#include "CBufferManager.h"
#include "InputLayout.h"
#include "ShaderCompiler.h"
#include "DepthStencil.h"

class DirectXCommon;
class WinApp;

class PipelineStateObject {
public:
	/// <summary>
	/// 初期化します
	/// </summary>
	void Initialize();

public:
	ID3D12PipelineState* GetPipelineState();
	ID3D12RootSignature* GetRootSignature();

public:
	DepthStencil* GetDepthStencil();

private:
	DirectXCommon* dxCommon_;
	WinApp* winApp_;
	CBufferManager cBufferManager_;
	InputLayout inputLayout_;
	ShaderCompiler shaderCompiler_;
	DepthStencil depthStencil_;
	ID3D12PipelineState* graphicsPipelineState_;

	ID3D10Blob* signatureBlob_;
	ID3D10Blob* errorBlob_;
	ID3D12RootSignature* rootSignature_;

};