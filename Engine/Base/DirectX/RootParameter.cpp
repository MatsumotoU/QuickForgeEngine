#include "RootParameter.h"
#include <cassert>

#ifdef _DEBUG
#include "Utility/MyDebugLog.h"
#include "Base/DirectX/DirectXStructToString.h"
#endif // _DEBUG


void RootParameter::Initialize() {
	rootParameters_.clear();

	// RootSignatureの生成
	descriptionRootSignature_ = {};
	descriptionRootSignature_.Flags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	friendlyNames_.clear();
}

void RootParameter::CreateRootParameter(const std::string& friendlyName, const D3D12_ROOT_PARAMETER_TYPE& parameterType, const D3D12_SHADER_VISIBILITY& shaderVisibility, int shaderRegisterIndex) {
	// RootParameter作成。
	D3D12_ROOT_PARAMETER rootParameters{};
	rootParameters_.push_back(rootParameters);
	rootParameters_[rootParameters_.size() - 1].ParameterType = parameterType;
	rootParameters_[rootParameters_.size() - 1].ShaderVisibility = shaderVisibility;
	rootParameters_[rootParameters_.size() - 1].Descriptor.ShaderRegister = shaderRegisterIndex;
	descriptionRootSignature_.pParameters = rootParameters_.data();
	descriptionRootSignature_.NumParameters = static_cast<UINT>(rootParameters_.size());

	// ルートシグネチャの登録名を保持
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

	descriptorRanges_[friendlyName] = descriptorRange; // マップに登録

#ifdef _DEBUG
	if (rootParameter->ParameterType != D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE) {
		DebugLog("RootParameter: ParameterType is Not TableType! ChangedType->D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE");
	}
#endif // _DEBUG

	// ルートパラメータのタイプをテーブルに設定
	rootParameter->ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameter->DescriptorTable.NumDescriptorRanges = 1; // 1つの範囲を持つ
	rootParameter->DescriptorTable.pDescriptorRanges = &descriptorRanges_[friendlyName];
}

#ifdef _DEBUG
void RootParameter::CheckIntegrityData() {
	DebugLog("RootParameter: CheckIntegrityData");

	// ルートパラメータが定義されていない場合はログを出力
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

	// ルートパラメータの名前を検索
	for (std::string& name : friendlyNames_) {
		if (name == friendlyName) {
			// 名前が一致した場合、対応するルートパラメータを取得
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
