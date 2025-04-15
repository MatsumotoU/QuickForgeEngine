#pragma once
#include <vector>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxgidebug.h>

class CBufferManager {
public:

	/// <summary>
	/// 初期化します
	/// </summary>
	void Initialize();
	/// <summary>
	/// RootParamaterを作成します
	/// </summary>
	/// <param name="parameterType">パラメータタイプ</param>
	/// <param name="shaderVisibility">シェーダーの種類</param>
	/// <param name="shaderRegisterIndex">レジスタ番号</param>
	void CreateRootParameter(const D3D12_ROOT_PARAMETER_TYPE& parameterType,const D3D12_SHADER_VISIBILITY& shaderVisibility,int shaderRegisterIndex);

	void CreateEmptyRootParameter();

public:
	std::vector<D3D12_ROOT_PARAMETER>* GetRootParameters();
	D3D12_ROOT_SIGNATURE_DESC* GetDescriptionRootSignature();

private:
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature_;
	std::vector<D3D12_ROOT_PARAMETER> rootParameters_;
};