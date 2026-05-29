#include "RootParameter.h"
#include <cassert>

#include "EngineDefines.h"

using namespace QFE;

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

	descriptorRanges_[friendlyName] = descriptorRange; // 郢晄ｧｭ繝｣郢晏干竊馴具ｽｻ鬪ｭ・ｲ

	if (rootParameter->ParameterType != D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE) {
		QFE_LOG("RootParameter: ParameterType is Not TableType! ChangedType->D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE");
	}

	// 郢晢ｽｫ郢晢ｽｼ郢晏現繝ｱ郢晢ｽｩ郢晢ｽ｡郢晢ｽｼ郢ｧ・ｿ邵ｺ・ｮ郢ｧ・ｿ郢ｧ・､郢晏干・堤ｹ昴・繝ｻ郢晄じﾎ晉ｸｺ・ｫ髫ｪ・ｭ陞ｳ繝ｻ
	rootParameter->ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameter->DescriptorTable.NumDescriptorRanges = 1; // 1邵ｺ・､邵ｺ・ｮ驕ｽ繝ｻ蟲・ｹｧ蜻域亜邵ｺ・､
	rootParameter->DescriptorTable.pDescriptorRanges = &descriptorRanges_[friendlyName];
}

D3D12_ROOT_PARAMETER* RootParameter::GetRootParameter(const std::string& friendlyName) {
	D3D12_ROOT_PARAMETER* result = nullptr;

	// 郢晢ｽｫ郢晢ｽｼ郢晏現繝ｱ郢晢ｽｩ郢晢ｽ｡郢晢ｽｼ郢ｧ・ｿ邵ｺ・ｮ陷ｷ讎顔√郢ｧ蜻茨ｽ､諛・ｽｴ・｢
	for (std::string& name : friendlyNames_) {
		if (name == friendlyName) {
			// 陷ｷ讎顔√邵ｺ蠕｡・ｸﾂ髢ｾ・ｴ邵ｺ蜉ｱ笳・撻・ｴ陷ｷ蛹ｻﾂ竏晢ｽｯ・ｾ陟｢諛岩・郢ｧ荵斟晉ｹ晢ｽｼ郢晏現繝ｱ郢晢ｽｩ郢晢ｽ｡郢晢ｽｼ郢ｧ・ｿ郢ｧ雋槫徐陟輔・
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


