#include "engine/include/graphic/Pipeline/PSO/RootParameter.h"
#include <cassert>

#ifdef QFE_OPTIMIZE_OFF
#include "engine/include/utility/DebugTool/DebugLog/MyDebugLog.h"
#include "engine/include/utility/String/DirectXStructToString.h"
#endif // QFE_OPTIMIZE_OFF
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
	// RootParameter闖ｴ諛医・邵ｲ繝ｻ
	D3D12_ROOT_PARAMETER rootParameters{};
	rootParameters_.push_back(rootParameters);
	rootParameters_[rootParameters_.size() - 1].ParameterType = parameterType;
	rootParameters_[rootParameters_.size() - 1].ShaderVisibility = shaderVisibility;
	rootParameters_[rootParameters_.size() - 1].Descriptor.ShaderRegister = shaderRegisterIndex;
	descriptionRootSignature_.pParameters = rootParameters_.begin();
	descriptionRootSignature_.NumParameters = static_cast<UINT>(rootParameters_.size());

	// 郢晢ｽｫ郢晢ｽｼ郢晏現縺咏ｹｧ・ｰ郢晞亂繝｡郢晢ｽ｣邵ｺ・ｮ騾具ｽｻ鬪ｭ・ｲ陷ｷ髦ｪ・定将譎・亜
	friendlyNames_.push_back(friendlyName);
}

void RootParameter::SetDescriptorRange(const std::string& friendlyName, const D3D12_DESCRIPTOR_RANGE_TYPE& rangeType, UINT numDescriptors, UINT baseShaderRegister) {
	D3D12_ROOT_PARAMETER* rootParameter = GetRootParameter(friendlyName);
	if (rootParameter == nullptr) {
#ifdef QFE_OPTIMIZE_OFF
		DebugLog("RootParameter: RootParameter not found for the given friendly name.");
#endif // QFE_OPTIMIZE_OFF
		assert(false && "RootParameter not found for the given friendly name.");
		return;
	}

	D3D12_DESCRIPTOR_RANGE descriptorRange{};
	descriptorRange.RangeType = rangeType;
	descriptorRange.NumDescriptors = numDescriptors;
	descriptorRange.BaseShaderRegister = baseShaderRegister;
	descriptorRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	descriptorRanges_[friendlyName] = descriptorRange; // 郢晄ｧｭ繝｣郢晏干竊馴具ｽｻ鬪ｭ・ｲ

#ifdef QFE_OPTIMIZE_OFF
	if (rootParameter->ParameterType != D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE) {
		DebugLog("RootParameter: ParameterType is Not TableType! ChangedType->D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE");
	}
#endif // QFE_OPTIMIZE_OFF

	// 郢晢ｽｫ郢晢ｽｼ郢晏現繝ｱ郢晢ｽｩ郢晢ｽ｡郢晢ｽｼ郢ｧ・ｿ邵ｺ・ｮ郢ｧ・ｿ郢ｧ・､郢晏干・堤ｹ昴・繝ｻ郢晄じﾎ晉ｸｺ・ｫ髫ｪ・ｭ陞ｳ繝ｻ
	rootParameter->ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameter->DescriptorTable.NumDescriptorRanges = 1; // 1邵ｺ・､邵ｺ・ｮ驕ｽ繝ｻ蟲・ｹｧ蜻域亜邵ｺ・､
	rootParameter->DescriptorTable.pDescriptorRanges = &descriptorRanges_[friendlyName];
}

#ifdef QFE_OPTIMIZE_OFF
void RootParameter::CheckIntegrityData() {
	DebugLog("RootParameter: CheckIntegrityData");

	// 郢晢ｽｫ郢晢ｽｼ郢晏現繝ｱ郢晢ｽｩ郢晢ｽ｡郢晢ｽｼ郢ｧ・ｿ邵ｺ謔滂ｽｮ螟ゑｽｾ・ｩ邵ｺ霈費ｽ檎ｸｺ・ｦ邵ｺ繝ｻ竊醍ｸｺ繝ｻ・ｰ・ｴ陷ｷ蛹ｻ繝ｻ郢晢ｽｭ郢ｧ・ｰ郢ｧ雋槭・陷峨・
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
#endif // QFE_OPTIMIZE_OFF

D3D12_ROOT_PARAMETER* RootParameter::GetRootParameter(const std::string& friendlyName) {
	D3D12_ROOT_PARAMETER* result = nullptr;

	// 郢晢ｽｫ郢晢ｽｼ郢晏現繝ｱ郢晢ｽｩ郢晢ｽ｡郢晢ｽｼ郢ｧ・ｿ邵ｺ・ｮ陷ｷ讎顔√郢ｧ蜻茨ｽ､諛・ｽｴ・｢
	for (std::string& name : friendlyNames_) {
		if (name == friendlyName) {
			// 陷ｷ讎顔√邵ｺ蠕｡・ｸﾂ髢ｾ・ｴ邵ｺ蜉ｱ笳・撻・ｴ陷ｷ蛹ｻﾂ竏晢ｽｯ・ｾ陟｢諛岩・郢ｧ荵斟晉ｹ晢ｽｼ郢晏現繝ｱ郢晢ｽｩ郢晢ｽ｡郢晢ｽｼ郢ｧ・ｿ郢ｧ雋槫徐陟輔・
			size_t index = &name - &friendlyNames_[0];
			assert(index < rootParameters_.size() && "Index out of bounds for root parameters.");
			result = &rootParameters_[index];
#ifdef QFE_OPTIMIZE_OFF
			DebugLog(std::format("RootParameter: Return {}", friendlyNames_[index]));
#endif // QFE_OPTIMIZE_OFF
			return result;
		} 
	}
	assert(false && "RootParameter not found for the given friendly name.");
	return result;
}

D3D12_ROOT_SIGNATURE_DESC* RootParameter::GetDescriptionRootSignature() {
	return &descriptionRootSignature_;
}


