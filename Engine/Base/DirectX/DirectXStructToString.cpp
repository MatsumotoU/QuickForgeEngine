#include "DirectXStructToString.h"
#ifdef _DEBUG
std::string DirectXStructToString::ToString(const D3D12_ROOT_PARAMETER& rootParameter) {
	std::string result = "D3D12_ROOT_PARAMETER: ";
	result += "ParameterType: " + std::to_string(rootParameter.ParameterType) + ", ";
	result += "ShaderVisibility: " + std::to_string(rootParameter.ShaderVisibility) + ", ";
	result += "Descriptor.ShaderRegister: " + std::to_string(rootParameter.Descriptor.ShaderRegister);
	return result;
}

std::string DirectXStructToString::ToString(const D3D12_DESCRIPTOR_RANGE& descriptorRange) {
	return "D3D12_DESCRIPTOR_RANGE: Type: " + std::to_string(descriptorRange.RangeType) +
		", NumDescriptors: " + std::to_string(descriptorRange.NumDescriptors) +
		", BaseShaderRegister: " + std::to_string(descriptorRange.BaseShaderRegister);
}
#endif // _DEBUG