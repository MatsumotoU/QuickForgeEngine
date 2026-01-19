#include "engine/include/graphic/Pipeline/PSO/RootParameter.h"
#include <cassert>

#ifdef _DEBUG
#include "engine/include/utility/DebugTool/DebugLog/MyDebugLog.h"
#include "engine/include/utility/String/DirectXStructToString.h"
#endif // _DEBUG


void RootParameter::Initialize() {
	// RootSignature
	descriptionRootSignature_ = {};
	descriptionRootSignature_.Flags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	rootParameters_.clear();
	friendlyNames_.clear();
}

void RootParameter::CreateRootParameter(const std::string& friendlyName, const D3D12_ROOT_PARAMETER_TYPE& parameterType, const D3D12_SHADER_VISIBILITY& shaderVisibility, int shaderRegisterIndex) {
	// RootParameter菴懈・縲・
	D3D12_ROOT_PARAMETER rootParameters{};
	rootParameters_.push_back(rootParameters);
	rootParameters_[rootParameters_.size() - 1].ParameterType = parameterType;
	rootParameters_[rootParameters_.size() - 1].ShaderVisibility = shaderVisibility;
	rootParameters_[rootParameters_.size() - 1].Descriptor.ShaderRegister = shaderRegisterIndex;
	descriptionRootSignature_.pParameters = rootParameters_.data();
	descriptionRootSignature_.NumParameters = static_cast<UINT>(rootParameters_.size());

	// 繝ｫ繝ｼ繝医す繧ｰ繝阪メ繝｣縺ｮ逋ｻ骭ｲ蜷阪ｒ菫晄戟
	friendlyNames_.push_back(friendlyName);
}

void RootParameter::SetDescriptorRange(const std::string& friendlyName, const D3D12_DESCRIPTOR_RANGE_TYPE& rangeType, UINT numDescriptors, UINT baseShaderRegister) {
	D3D12_ROOT_PARAMETER* rootParameter = GetRootParameter(friendlyName);
	if (rootParameter == nullptr) {
#ifdef _DEBUG
		DebugLog("RootParameter: RootParameter not found for the given friendly name.");
#endif // _DEBUG
		assert(false && "RootParameter not found for the given friendly name.");
		return;
	}

	D3D12_DESCRIPTOR_RANGE descriptorRange{};
	descriptorRange.RangeType = rangeType;
	descriptorRange.NumDescriptors = numDescriptors;
	descriptorRange.BaseShaderRegister = baseShaderRegister;
	descriptorRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	descriptorRanges_[friendlyName] = descriptorRange; // 繝槭ャ繝励↓逋ｻ骭ｲ

#ifdef _DEBUG
	if (rootParameter->ParameterType != D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE) {
		DebugLog("RootParameter: ParameterType is Not TableType! ChangedType->D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE");
	}
#endif // _DEBUG

	// 繝ｫ繝ｼ繝医ヱ繝ｩ繝｡繝ｼ繧ｿ縺ｮ繧ｿ繧､繝励ｒ繝・・繝悶Ν縺ｫ險ｭ螳・
	rootParameter->ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameter->DescriptorTable.NumDescriptorRanges = 1; // 1縺､縺ｮ遽・峇繧呈戟縺､
	rootParameter->DescriptorTable.pDescriptorRanges = &descriptorRanges_[friendlyName];
}

#ifdef _DEBUG
void RootParameter::CheckIntegrityData() {
	DebugLog("RootParameter: CheckIntegrityData");

	// 繝ｫ繝ｼ繝医ヱ繝ｩ繝｡繝ｼ繧ｿ縺悟ｮ夂ｾｩ縺輔ｌ縺ｦ縺・↑縺・ｴ蜷医・繝ｭ繧ｰ繧貞・蜉・
	if (rootParameters_.empty()) {
		DebugLog("RootParameter: No root parameters defined.");
		return;
	}
	DebugLog("RootParameter: Integrity check completed successfully.");

	DebugLog("RootParameter: ===ParameterList===");
	for (const std::string& name : friendlyNames_) {
		D3D12_ROOT_PARAMETER* rootParameter = GetRootParameter(name);
		DebugLog(DirectXStructToString::ToString(*rootParameter));
	}
}
#endif // _DEBUG

D3D12_ROOT_PARAMETER* RootParameter::GetRootParameter(const std::string& friendlyName) {
	D3D12_ROOT_PARAMETER* result = nullptr;

	// 繝ｫ繝ｼ繝医ヱ繝ｩ繝｡繝ｼ繧ｿ縺ｮ蜷榊燕繧呈､懃ｴ｢
	for (std::string& name : friendlyNames_) {
		if (name == friendlyName) {
			// 蜷榊燕縺御ｸ閾ｴ縺励◆蝣ｴ蜷医∝ｯｾ蠢懊☆繧九Ν繝ｼ繝医ヱ繝ｩ繝｡繝ｼ繧ｿ繧貞叙蠕・
			size_t index = &name - &friendlyNames_[0];
			assert(index < rootParameters_.size() && "Index out of bounds for root parameters.");
			result = &rootParameters_[index];
#ifdef _DEBUG
			DebugLog(std::format("RootParameter: Return {}", friendlyNames_[index]));
#endif // _DEBUG
			return result;
		} 
	}
	assert(false && "RootParameter not found for the given friendly name.");
	return result;
}

std::vector<D3D12_ROOT_PARAMETER>* RootParameter::GetRootParameters() {
	return &rootParameters_;
}

D3D12_ROOT_SIGNATURE_DESC* RootParameter::GetDescriptionRootSignature() {
	return &descriptionRootSignature_;
}
