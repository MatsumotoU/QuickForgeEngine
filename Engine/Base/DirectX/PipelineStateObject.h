#pragma once
#include "RootParameter.h"
#include "InputLayout.h"
#include "ShaderCompiler.h"
#include <wrl.h>

class DepthStencil;
class EngineCore;
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

class PipelineStateObject final {
public:
	/// <summary>
	/// 初期化します
	/// </summary>
	void Initialize(EngineCore* engineCore);

	void CreatePipelineStateObject(
		RootParameter rootParameter, DepthStencil* depthStencil, const D3D12_PRIMITIVE_TOPOLOGY_TYPE& topologyType,
		D3D12_FILL_MODE fillMode, const std::string& psFilepath, const std::string& vsFilepath, BlendMode blendMode, bool isDrawBack);

public:
	ID3D12PipelineState* GetPipelineState();
	ID3D12RootSignature* GetRootSignature();

public:
	DepthStencil* GetDepthStencil();

private:
	EngineCore* engineCore_;
	DirectXCommon* dxCommon_;
	WinApp* winApp_;
	DepthStencil* depthStencil_;
	InputLayout inputLayout_;
	ShaderCompiler* shaderCompiler_;
	
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState_;

	Microsoft::WRL::ComPtr<ID3D10Blob> signatureBlob_;
	Microsoft::WRL::ComPtr<ID3D10Blob> errorBlob_;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
};