#pragma once
#include <vector>
#include <string>
#include<map>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxgidebug.h>

/**
 * @class RootParameter
 * @brief ルートシグネチャのパラメータを管理するクラス
 */
class RootParameter final {
public:
	/// @brief 初期化します
	void Initialize();

	/// @brief RootParamaterを作成します
	void CreateRootParameter(
		const std::string& friendlyName,
		const D3D12_ROOT_PARAMETER_TYPE& parameterType,
		const D3D12_SHADER_VISIBILITY& shaderVisibility,
		int shaderRegisterIndex);
	/// @brief ディスクリプターレンジを設定します
	void SetDescriptorRange(
		const std::string& friendlyName,
		const D3D12_DESCRIPTOR_RANGE_TYPE& rangeType,
		UINT numDescriptors, UINT baseShaderRegister);

#ifdef _DEBUG
	void CheckIntegrityData();
#endif // _DEBUG
	
public:
	D3D12_ROOT_PARAMETER* GetRootParameter(const std::string& friendlyName);
	std::vector<D3D12_ROOT_PARAMETER>* GetRootParameters();
	D3D12_ROOT_SIGNATURE_DESC* GetDescriptionRootSignature();

private:

	std::map<std::string, D3D12_DESCRIPTOR_RANGE> descriptorRanges_; // ルートパラメータの登録名とディスクリプタレンジのマップ
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature_;
	std::vector<D3D12_ROOT_PARAMETER> rootParameters_;
	std::vector<std::string> friendlyNames_; // ルートシグネチャの登録名を保持するためのベクター
};
