#pragma once
#include <d3d12.h>
#include <wrl.h>

#include "ShaderCompiler.h"
#include "ShaderReflection.h"

class PSOCreater final{
public:
	/// <summary>
	/// パイプラインステートオブジェクトを生成します
	/// </summary>
	/// <param name="device">D3D12デバイス</param>
	/// <param name="psoDesc">パイプラインステートの設定</param>
	/// <returns>生成されたパイプラインステートオブジェクト</returns>
	Microsoft::WRL::ComPtr<ID3D12PipelineState> CreatePSO(ID3D12Device* device, const D3D12_GRAPHICS_PIPELINE_STATE_DESC& psoDesc);
};