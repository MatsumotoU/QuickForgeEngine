#include "engine/include/utility/String/DirectXStructToString.h"
#ifdef QFE_OPTIMIZE_OFF
#include <string>

namespace QFE {
	namespace DirectXStructToString {

		std::string ToString(const D3D12_ROOT_PARAMETER& rootParameter) {
			std::string result = "D3D12_ROOT_PARAMETER: ";
			result += "ParameterType: " + std::to_string(rootParameter.ParameterType) + ", ";
			result += "ShaderVisibility: " + std::to_string(rootParameter.ShaderVisibility) + ", ";
			result += "Descriptor.ShaderRegister: " + std::to_string(rootParameter.Descriptor.ShaderRegister);
			return result;
		}

		std::string ToString(const D3D12_DESCRIPTOR_RANGE& descriptorRange) {
			return "D3D12_DESCRIPTOR_RANGE: Type: " + std::to_string(descriptorRange.RangeType) +
				", NumDescriptors: " + std::to_string(descriptorRange.NumDescriptors) +
				", BaseShaderRegister: " + std::to_string(descriptorRange.BaseShaderRegister);
		}

	}
}
#endif // QFE_OPTIMIZE_OFF
