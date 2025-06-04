#include "CBufferManager.h"

// TODO: 名前の変更

void CBufferManager::Initialize() {
	rootParameters_.clear();

	// RootSignatureの生成
	descriptionRootSignature_ = {};
	descriptionRootSignature_.Flags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
}

void CBufferManager::CreateRootParameter(const D3D12_ROOT_PARAMETER_TYPE& parameterType, const D3D12_SHADER_VISIBILITY& shaderVisibility, int shaderRegisterIndex) {
	// RootParameter作成。複数設定できるので配列。今回は結果1つだけなので長さ1の配列
	D3D12_ROOT_PARAMETER rootParameters{};
	rootParameters_.push_back(rootParameters);
	rootParameters_[rootParameters_.size() - 1].ParameterType = parameterType;
	rootParameters_[rootParameters_.size() - 1].ShaderVisibility = shaderVisibility;
	rootParameters_[rootParameters_.size() - 1].Descriptor.ShaderRegister = shaderRegisterIndex;
	descriptionRootSignature_.pParameters = rootParameters_.data();
	descriptionRootSignature_.NumParameters = static_cast<UINT>(rootParameters_.size());
}

void CBufferManager::CreateEmptyRootParameter() {
	D3D12_ROOT_PARAMETER rootParameters{};
	rootParameters_.push_back(rootParameters);
}

std::vector<D3D12_ROOT_PARAMETER>* CBufferManager::GetRootParameters() {
	return &rootParameters_;
}

D3D12_ROOT_SIGNATURE_DESC* CBufferManager::GetDescriptionRootSignature() {
	return &descriptionRootSignature_;
}
