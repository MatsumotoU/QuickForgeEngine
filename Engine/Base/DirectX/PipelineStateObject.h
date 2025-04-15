#pragma once
#include "CBufferManager.h"
#include "InputLayout.h"
#include "ShaderCompiler.h"

class DirectXCommon;

class PipelineStateObject {
public:
	/// <summary>
	/// 初期化します
	/// </summary>
	void Initialize();

public:
	ID3D12PipelineState* GetPipelineState();
	ID3D12RootSignature* GetRootSignature();

private:
	DirectXCommon* dxCommon_;
	CBufferManager cBufferManager_;
	InputLayout inputLayout_;
	ShaderCompiler shaderCompiler_;
	ID3D12PipelineState* graphicsPipelineState_;

	ID3D10Blob* signatureBlob_;
	ID3D10Blob* errorBlob_;
	ID3D12RootSignature* rootSignature_;

};