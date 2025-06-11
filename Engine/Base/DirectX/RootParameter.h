#pragma once
#include <vector>
#include <string>
#include<map>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxgidebug.h>

class RootParameter final {
public:

	/// <summary>
	/// 初期化します
	/// </summary>
	void Initialize();
	/// <summary>
	/// RootParamaterを作成します
	/// </summary>
	/// <param name="friendlyName">ルートシグネチャの登録名</param>
	/// <param name="parameterType">パラメータタイプ</param>
	/// <param name="shaderVisibility">シェーダーの種類</param>
	/// <param name="shaderRegisterIndex">レジスタ番号</param>
	void CreateRootParameter(const std::string& friendlyName,const D3D12_ROOT_PARAMETER_TYPE& parameterType, const D3D12_SHADER_VISIBILITY& shaderVisibility, int shaderRegisterIndex);
	/// <summary>
	/// ディスクリプターレンジを設定します
	/// </summary>
	/// <param name="friendlyName"></param>
	/// <param name="rangeType"></param>
	/// <param name="numDescriptors"></param>
	/// <param name="baseShaderRegister"></param>
	void SetDescriptorRange(
		const std::string& friendlyName, const D3D12_DESCRIPTOR_RANGE_TYPE& rangeType, UINT numDescriptors, UINT baseShaderRegister);

#ifdef _DEBUG
	void CheckIntegrityData();
#endif // _DEBUG
	
public:
	D3D12_ROOT_PARAMETER* GetRootParameter(const std::string& friendlyName);
	std::vector<D3D12_ROOT_PARAMETER>* GetRootParameters();
	D3D12_ROOT_SIGNATURE_DESC* GetDescriptionRootSignature();

private:

	std::map<std::string, D3D12_DESCRIPTOR_RANGE> descriptorRanges_; // ルートパラメータの登録名と範囲を保持するマップ
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature_;
	std::vector<D3D12_ROOT_PARAMETER> rootParameters_;
	std::vector<std::string> friendlyNames_; // ルートシグネチャの登録名を保持するためのベクター
};