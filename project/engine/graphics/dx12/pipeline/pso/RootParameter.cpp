#include "RootParameter.h"
#include <cassert>

#include "EngineDefines.h"

using namespace QFE::GRAPHIC;

void RootParameter::Initialize(D3D12_ROOT_SIGNATURE_FLAGS flags) {
	// RootSignature
	descriptionRootSignature_ = {};
	descriptionRootSignature_.Flags = flags;

	rootParameters_.clear();
	friendlyNames_.clear();
}

void RootParameter::CreateRootParameter(const std::string& friendlyName, const D3D12_ROOT_PARAMETER_TYPE& parameterType, const D3D12_SHADER_VISIBILITY& shaderVisibility, int shaderRegisterIndex) {
	// RootParameterの追加
	D3D12_ROOT_PARAMETER rootParameters{};
	rootParameters_.push_back(rootParameters);
	rootParameters_[rootParameters_.size() - 1].ParameterType = parameterType;
	rootParameters_[rootParameters_.size() - 1].ShaderVisibility = shaderVisibility;
	rootParameters_[rootParameters_.size() - 1].Descriptor.ShaderRegister = static_cast<UINT>(shaderRegisterIndex);
	descriptionRootSignature_.pParameters = rootParameters_.begin();
	descriptionRootSignature_.NumParameters = static_cast<UINT>(rootParameters_.size());

	// RootParameterの名前を管理するためのベクターにfriendlyNameを追加
	friendlyNames_.push_back(friendlyName);
}

void RootParameter::SetDescriptorRange(const std::string& friendlyName, const D3D12_DESCRIPTOR_RANGE_TYPE& rangeType, UINT numDescriptors, UINT baseShaderRegister) {
	D3D12_ROOT_PARAMETER* rootParameter = GetRootParameter(friendlyName);
	if (rootParameter == nullptr) {
		QFE_LOG("RootParameter: RootParameter not found for the given friendly name.");
		assert(false && "RootParameter not found for the given friendly name.");
		return;
	}

	D3D12_DESCRIPTOR_RANGE descriptorRange{};
	descriptorRange.RangeType = rangeType;
	descriptorRange.NumDescriptors = numDescriptors;
	descriptorRange.BaseShaderRegister = baseShaderRegister;
	descriptorRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	descriptorRanges_[friendlyName] = descriptorRange;// friendlyNameをキーにして、対応するDescriptorRangeを保存

	if (rootParameter->ParameterType != D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE) {
		QFE_LOG("RootParameter: ParameterType is Not TableType! ChangedType->D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE");
	}

	// RootParameterのParameterTypeをD3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLEに設定し、DescriptorTableのNumDescriptorRangesとpDescriptorRangesを設定
	rootParameter->ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameter->DescriptorTable.NumDescriptorRanges = 1; // 1つのDescriptorRangeを設定するため、NumDescriptorRangesは1に設定
	rootParameter->DescriptorTable.pDescriptorRanges = &descriptorRanges_[friendlyName];
}

void QFE::GRAPHIC::RootParameter::CreateRootParameter(const RootParameterElement& rootParameterElement, const D3D12_SHADER_VISIBILITY& shaderVisibility) {
	if (rootParameterElement.shaderInputType == D3D_SIT_CBUFFER) {

		// ルート定数バッファとしてルートパラメータを作成
		CreateRootParameter(
			rootParameterElement.friendlyName,
			D3D12_ROOT_PARAMETER_TYPE_CBV,
			shaderVisibility,
			rootParameterElement.shaderRegisterIndex);

	} else if (rootParameterElement.shaderInputType == D3D_SIT_TEXTURE ||
		rootParameterElement.shaderInputType == D3D_SIT_STRUCTURED) {

		// ディスクリプタテーブルとしてルートパラメータを作成
		CreateRootParameter(
			rootParameterElement.friendlyName,
			D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
			shaderVisibility,
			rootParameterElement.shaderRegisterIndex);
		// SRVのディスクリプタレンジを設定
		D3D12_DESCRIPTOR_RANGE_TYPE rangeType;
		rangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		SetDescriptorRange(
			rootParameterElement.friendlyName,
			rangeType,
			static_cast<UINT>(rootParameterElement.descriptorCount),
			static_cast<UINT>(rootParameterElement.shaderRegisterIndex));

	} else if (rootParameterElement.shaderInputType == D3D_SIT_UAV_RWTYPED ||
		rootParameterElement.shaderInputType == D3D_SIT_UAV_RWSTRUCTURED ||
		rootParameterElement.shaderInputType == D3D_SIT_UAV_RWBYTEADDRESS) {

		// ディスクリプタテーブルとしてルートパラメータを作成
		CreateRootParameter(
			rootParameterElement.friendlyName,
			D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
			shaderVisibility,
			rootParameterElement.shaderRegisterIndex);
		// UAVのディスクリプタレンジを設定
		SetDescriptorRange(
			rootParameterElement.friendlyName,
			D3D12_DESCRIPTOR_RANGE_TYPE_UAV,
			static_cast<UINT>(rootParameterElement.descriptorCount),
			static_cast<UINT>(rootParameterElement.shaderRegisterIndex));

	}else if (rootParameterElement.shaderInputType == D3D_SIT_SAMPLER) {
		// サンプラーは静的サンプラーを使用するので、なにもしません
	}else if (rootParameterElement.shaderInputType == D3D_SIT_RTACCELERATIONSTRUCTURE) { // ★追加：レイトレ加速構造の場合

		// ルートSRV（Descriptor Tableではなく、Root Descriptor型）としてルートパラメータを作成
		CreateRootParameter(
			rootParameterElement.friendlyName,
			D3D12_ROOT_PARAMETER_TYPE_SRV, // 直接GPUアドレスを渡せるSRVタイプ
			shaderVisibility,
			rootParameterElement.shaderRegisterIndex);

	} else {
		QFE_LOG(std::format("RootParameter: Unsupported shader input type for friendly name '{}'.", rootParameterElement.friendlyName));
		assert(false && "Unsupported shader input type for the given friendly name.");
	}
}

void QFE::GRAPHIC::RootParameter::AssignStaticSampler(const D3D12_STATIC_SAMPLER_DESC* staticSamplerDescs, const UINT& size) {
	descriptionRootSignature_.pStaticSamplers = staticSamplerDescs;
	descriptionRootSignature_.NumStaticSamplers = size;
}

D3D12_ROOT_PARAMETER* RootParameter::GetRootParameter(const std::string& friendlyName) {
	D3D12_ROOT_PARAMETER* result = nullptr;
	// friendlyNames_からfriendlyNameを検索して、対応するRootParameterを返す
	for (size_t index = 0; index < friendlyNames_.size(); ++index) {
		const std::string& name = friendlyNames_[index];
		if (name == friendlyName) {
			// friendlyNameが見つかった場合、そのインデックスを使用してrootParameters_から対応するRootParameterを取得
			assert(index < rootParameters_.size() && "Index out of bounds for root parameters.");
			result = &rootParameters_[index];

			QFE_LOG(std::format("RootParameter: Return {}", friendlyNames_[index]));

			return result;
		} 
	}
	assert(false && "RootParameter not found for the given friendly name.");
	return result;
}

UINT RootParameter::GetRootParameterIndex(const std::string& friendlyName) const {
	for (size_t index = 0; index < friendlyNames_.size(); ++index) {
		if (friendlyNames_[index] == friendlyName) {
			return static_cast<UINT>(index);
		}
	}

	QFE_LOG(std::format("RootParameter: '{}' was not found.", friendlyName));
	assert(false && "RootParameter not found for the given friendly name.");
	return static_cast<UINT>(-1);
}

D3D12_ROOT_SIGNATURE_DESC* RootParameter::GetDescriptionRootSignature() {
	return &descriptionRootSignature_;
}

std::vector<D3D12_ROOT_PARAMETER_TYPE> QFE::GRAPHIC::RootParameter::GetRootParameterTypes() const {
	std::vector<D3D12_ROOT_PARAMETER_TYPE> parameterTypes;
	for (const auto& rootParameter : rootParameters_) {
		parameterTypes.push_back(rootParameter.ParameterType);
		QFE_LOG(std::format("RootParameter: RootParameterType is {}", static_cast<int>(rootParameter.ParameterType)));
	}
	return parameterTypes;
}

std::vector<ViewTypeFlags> QFE::GRAPHIC::RootParameter::GetRootParameterViewTypes() const {
	std::vector<ViewTypeFlags> viewTypes;
	viewTypes.reserve(rootParameters_.size());
	for (const D3D12_ROOT_PARAMETER& rootParameter : rootParameters_) {
		switch (rootParameter.ParameterType) {
		case D3D12_ROOT_PARAMETER_TYPE_CBV:
			viewTypes.push_back(ViewTypeFlags::ConstantBufferView);
			break;
		case D3D12_ROOT_PARAMETER_TYPE_SRV:
			viewTypes.push_back(ViewTypeFlags::ShaderResourceView);
			break;
		case D3D12_ROOT_PARAMETER_TYPE_UAV:
			viewTypes.push_back(ViewTypeFlags::UnorderedAccessView);
			break;
		case D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE:
			if (rootParameter.DescriptorTable.NumDescriptorRanges == 0 ||
				rootParameter.DescriptorTable.pDescriptorRanges == nullptr) {
				viewTypes.push_back(ViewTypeFlags::None);
				break;
			}
			switch (rootParameter.DescriptorTable.pDescriptorRanges[0].RangeType) {
			case D3D12_DESCRIPTOR_RANGE_TYPE_CBV:
				viewTypes.push_back(ViewTypeFlags::ConstantBufferView);
				break;
			case D3D12_DESCRIPTOR_RANGE_TYPE_SRV:
				viewTypes.push_back(ViewTypeFlags::ShaderResourceView);
				break;
			case D3D12_DESCRIPTOR_RANGE_TYPE_UAV:
				viewTypes.push_back(ViewTypeFlags::UnorderedAccessView);
				break;
			case D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER:
				viewTypes.push_back(ViewTypeFlags::None);
				break;
			default:
				viewTypes.push_back(ViewTypeFlags::None);
				break;
			}
			break;
		case D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS:
			viewTypes.push_back(ViewTypeFlags::None);
			break;
		default:
			viewTypes.push_back(ViewTypeFlags::None);
			break;
		}
	}
	return viewTypes;
}

#ifdef QFE_OPTIMIZE_OFF
void RootParameter::CheckIntegrityData() {
	assert(descriptionRootSignature_.NumParameters == static_cast<UINT>(rootParameters_.size()));
	assert(rootParameters_.size() == friendlyNames_.size());

	for (const auto& name : friendlyNames_) {
		assert(!name.empty());

		const bool hasRange = descriptorRanges_.find(name) != descriptorRanges_.end();
		if (hasRange) {
			D3D12_ROOT_PARAMETER* rootParameter = GetRootParameter(name);
			if (rootParameter != nullptr) {
				QFE_REPORT_SYSTEM_ERROR("RootParameter: RootParameter not found for the given friendly name.", SystemError::Abort);
			}
			if(rootParameter->ParameterType != D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE) {
				QFE_REPORT_SYSTEM_ERROR("RootParameter: ParameterType is Not TableType!", SystemError::Abort);
			}
			if(rootParameter->DescriptorTable.NumDescriptorRanges <= 0) {
				QFE_REPORT_SYSTEM_ERROR("RootParameter: NumDescriptorRanges is less than or equal to 0!", SystemError::Abort);
			}
		}
	}
}
#endif // QFE_OPTIMIZE_OFF


