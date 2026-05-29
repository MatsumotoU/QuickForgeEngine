#pragma once
#include <string>
#include <map>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxgidebug.h>

#include "memory/SafeVector.h"

namespace QFE {
	/**
	 * @class RootParameter
	 * @brief 
	 */
	class RootParameter final {
	public:
		/// @brief 初期化処理
		void Initialize();

		/// @brief RootParamaterを追加する
		void CreateRootParameter(
			const std::string& friendlyName,
			const D3D12_ROOT_PARAMETER_TYPE& parameterType,
			const D3D12_SHADER_VISIBILITY& shaderVisibility,
			int shaderRegisterIndex);
		/// @brief DescriptorRangeを設定する
		void SetDescriptorRange(
			const std::string& friendlyName,
			const D3D12_DESCRIPTOR_RANGE_TYPE& rangeType,
			UINT numDescriptors, UINT baseShaderRegister);

#ifdef QFE_OPTIMIZE_OFF
		void CheckIntegrityData();
#endif // QFE_OPTIMIZE_OFF

	public:
		D3D12_ROOT_PARAMETER* GetRootParameter(const std::string& friendlyName);
		D3D12_ROOT_SIGNATURE_DESC* GetDescriptionRootSignature();

	private:

		std::map<std::string, D3D12_DESCRIPTOR_RANGE> descriptorRanges_; // DescriptorRangeを管理するためのマップ。friendlyNameをキーにして、対応するDescriptorRangeを保存。
		D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature_;
		SafeVector<D3D12_ROOT_PARAMETER> rootParameters_;
		SafeVector<std::string> friendlyNames_; // RootParameterの名前を管理するためのベクター。friendlyNameをキーにして、対応するRootParameterを保存。
	};

}
