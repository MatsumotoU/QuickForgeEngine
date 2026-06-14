#pragma once
#include <string>
#include <map>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxgidebug.h>

#include "memory/SafeVector.h"
#include "RootParameterElement.h"

namespace QFE::GRAPHIC::INTERNAL {
	/// @brief ルートパラメータを管理するクラス
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

		/// @brief RootParameterをついかします
		void CreateRootParameter(const RootParameterElement& rootParameterElement, const D3D12_SHADER_VISIBILITY& shaderVisibility);
		/// @brief 静的サンプラーをルートシグネチャに割り当てます
		void AssignStaticSampler(const D3D12_STATIC_SAMPLER_DESC* staticSamplerDescs,const UINT& size);

#ifdef QFE_OPTIMIZE_OFF
		void CheckIntegrityData();
#endif // QFE_OPTIMIZE_OFF

	public:
		D3D12_ROOT_PARAMETER* GetRootParameter(const std::string& friendlyName);
		D3D12_ROOT_SIGNATURE_DESC* GetDescriptionRootSignature();

		/// @brief 登録順にRootParameterのタイプを取得します
		std::vector< D3D12_ROOT_PARAMETER_TYPE> GetRootParameterTypes() const;

	private:

		std::map<std::string, D3D12_DESCRIPTOR_RANGE> descriptorRanges_; // DescriptorRangeを管理するためのマップ。friendlyNameをキーにして、対応するDescriptorRangeを保存。
		D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature_;
		SafeVector<D3D12_ROOT_PARAMETER> rootParameters_;
		SafeVector<std::string> friendlyNames_; // RootParameterの名前を管理するためのベクター。friendlyNameをキーにして、対応するRootParameterを保存。
	};

}
