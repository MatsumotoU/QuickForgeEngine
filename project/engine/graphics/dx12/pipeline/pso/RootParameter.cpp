#include "RootParameter.h"
#include <cassert>

#include "EngineDefines.h"

using namespace QFE::GRAPHIC::INTERNAL;

void RootParameter::Initialize() {
	// RootSignature
	descriptionRootSignature_ = {};
	descriptionRootSignature_.Flags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	rootParameters_.clear();
	friendlyNames_.clear();
}

void RootParameter::CreateRootParameter(const std::string& friendlyName, const D3D12_ROOT_PARAMETER_TYPE& parameterType, const D3D12_SHADER_VISIBILITY& shaderVisibility, int shaderRegisterIndex) {
	// RootParameterの追加
	D3D12_ROOT_PARAMETER rootParameters{};
	rootParameters_.push_back(rootParameters);
	rootParameters_[rootParameters_.size() - 1].ParameterType = parameterType;
	rootParameters_[rootParameters_.size() - 1].ShaderVisibility = shaderVisibility;
	rootParameters_[rootParameters_.size() - 1].Descriptor.ShaderRegister = shaderRegisterIndex;
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

void QFE::GRAPHIC::INTERNAL::RootParameter::CreateRootParameter(const RootParameterElement& rootParameterElement, const D3D12_SHADER_VISIBILITY& shaderVisibility) {
	if (rootParameterElement.shaderInputType == D3D_SIT_CBUFFER) {
		CreateRootParameter(
			rootParameterElement.friendlyName,
			D3D12_ROOT_PARAMETER_TYPE_CBV,
			shaderVisibility,
			rootParameterElement.shaderRegisterIndex);
	}else if (rootParameterElement.shaderInputType == D3D_SIT_TEXTURE || rootParameterElement.shaderInputType == D3D_SIT_SAMPLER) {
		CreateRootParameter(
			rootParameterElement.friendlyName,
			D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
			shaderVisibility,
			rootParameterElement.shaderRegisterIndex);
		D3D12_DESCRIPTOR_RANGE_TYPE rangeType;
		if (rootParameterElement.shaderInputType == D3D_SIT_TEXTURE) {
			rangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		} else {
			rangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
		}
		SetDescriptorRange(
			rootParameterElement.friendlyName,
			rangeType,
			1, // numDescriptorsは1に設定
			rootParameterElement.shaderRegisterIndex);
	} else {
		QFE_LOG(std::format("RootParameter: Unsupported shader input type for friendly name '{}'.", rootParameterElement.friendlyName));
		assert(false && "Unsupported shader input type for the given friendly name.");
	}
}

D3D12_ROOT_PARAMETER* RootParameter::GetRootParameter(const std::string& friendlyName) {
	D3D12_ROOT_PARAMETER* result = nullptr;
	// friendlyNames_からfriendlyNameを検索して、対応するRootParameterを返す
	for (std::string& name : friendlyNames_) {
		if (name == friendlyName) {
			// friendlyNameが見つかった場合、そのインデックスを使用してrootParameters_から対応するRootParameterを取得
			size_t index = &name - &friendlyNames_[0];
			assert(index < rootParameters_.size() && "Index out of bounds for root parameters.");
			result = &rootParameters_[index];

			QFE_LOG(std::format("RootParameter: Return {}", friendlyNames_[index]));

			return result;
		} 
	}
	assert(false && "RootParameter not found for the given friendly name.");
	return result;
}

D3D12_ROOT_SIGNATURE_DESC* RootParameter::GetDescriptionRootSignature() {
	return &descriptionRootSignature_;
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
			assert(rootParameter != nullptr);
			assert(rootParameter->ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE);
			assert(rootParameter->DescriptorTable.NumDescriptorRanges > 0);
		}
	}
}
#endif // QFE_OPTIMIZE_OFF


