#pragma once
#include <vector>
#include <string>
#include<map>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxgidebug.h>

/**
 * @class RootParameter
 * @brief 繝ｫ繝ｼ繝医す繧ｰ繝阪メ繝｣縺ｮ繝代Λ繝｡繝ｼ繧ｿ繧堤ｮ｡逅・☆繧九け繝ｩ繧ｹ
 */
class RootParameter final {
public:
	/// @brief 蛻晄悄蛹悶＠縺ｾ縺・
	void Initialize();

	/// @brief RootParamater繧剃ｽ懈・縺励∪縺・
	void CreateRootParameter(
		const std::string& friendlyName,
		const D3D12_ROOT_PARAMETER_TYPE& parameterType,
		const D3D12_SHADER_VISIBILITY& shaderVisibility,
		int shaderRegisterIndex);
	/// @brief 繝・ぅ繧ｹ繧ｯ繝ｪ繝励ち繝ｼ繝ｬ繝ｳ繧ｸ繧定ｨｭ螳壹＠縺ｾ縺・
	void SetDescriptorRange(
		const std::string& friendlyName,
		const D3D12_DESCRIPTOR_RANGE_TYPE& rangeType,
		UINT numDescriptors, UINT baseShaderRegister);

#ifdef QFE_OPTIMIZE_OFF
	void CheckIntegrityData();
#endif // QFE_OPTIMIZE_OFF
	
public:
	D3D12_ROOT_PARAMETER* GetRootParameter(const std::string& friendlyName);
	std::vector<D3D12_ROOT_PARAMETER>* GetRootParameters();
	D3D12_ROOT_SIGNATURE_DESC* GetDescriptionRootSignature();

private:

	std::map<std::string, D3D12_DESCRIPTOR_RANGE> descriptorRanges_; // 繝ｫ繝ｼ繝医ヱ繝ｩ繝｡繝ｼ繧ｿ縺ｮ逋ｻ骭ｲ蜷阪→繝・ぅ繧ｹ繧ｯ繝ｪ繝励ち繝ｬ繝ｳ繧ｸ縺ｮ繝槭ャ繝・
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature_;
	std::vector<D3D12_ROOT_PARAMETER> rootParameters_;
	std::vector<std::string> friendlyNames_; // 繝ｫ繝ｼ繝医す繧ｰ繝阪メ繝｣縺ｮ逋ｻ骭ｲ蜷阪ｒ菫晄戟縺吶ｋ縺溘ａ縺ｮ繝吶け繧ｿ繝ｼ
};


