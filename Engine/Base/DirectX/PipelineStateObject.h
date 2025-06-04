#pragma once
#include "CBufferManager.h"
#include "InputLayout.h"
#include "ShaderCompiler.h"
#include <wrl.h>

class DepthStencil;
class DirectXCommon;
class WinApp;

// TODO: ブレンドモードの列挙型をブレンドモードを管理するクラスに移送
enum BlendMode{
	// ブレンドなし
	kBlendModeNone,
	// 通常
	kBlendModeNormal,
	// 加算
	kBlendModeAdd,
	// 減算
	kBlendModeSubtract,
	// 乗算
	kBlendModeMultily,
	// スクリーン
	kBlendModeScreen,
	// 利用禁止
	kCountOfBlendMode,
};

class PipelineStateObject {
public:
	/// <summary>
	/// 初期化します
	/// </summary>
	void Initialize(
		DirectXCommon* dxCommon, WinApp* winApp, DepthStencil* depthStencil,
		const D3D12_PRIMITIVE_TOPOLOGY_TYPE& topologyType, D3D12_FILL_MODE fillMode,const std::string& psFilename,BlendMode blendMode,bool isParticle);

public:
	ID3D12PipelineState* GetPipelineState();
	ID3D12RootSignature* GetRootSignature();

public:
	DepthStencil* GetDepthStencil();

private:
	DirectXCommon* dxCommon_;
	WinApp* winApp_;
	DepthStencil* depthStencil_;
	CBufferManager cBufferManager_;
	InputLayout inputLayout_;
	ShaderCompiler shaderCompiler_;
	
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState_;

	Microsoft::WRL::ComPtr<ID3D10Blob> signatureBlob_;
	Microsoft::WRL::ComPtr<ID3D10Blob> errorBlob_;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
};