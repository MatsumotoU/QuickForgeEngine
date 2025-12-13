#pragma once
#ifdef _DEBUG
#include <string>
#include <d3d12.h>

namespace DirectXStructToString {
	/// <summary>
	/// RootParameterやDescriptorRangeの冁E��を文字�Eに変換します、E
	/// </summary>
	std::string ToString(const D3D12_ROOT_PARAMETER& rootParameter);
	std::string ToString(const D3D12_DESCRIPTOR_RANGE& descriptorRange);
}
#endif // _DEBUG
