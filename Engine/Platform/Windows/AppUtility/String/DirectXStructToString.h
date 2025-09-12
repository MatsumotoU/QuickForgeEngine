#pragma once
#ifdef _DEBUG
#include <string>
#include <d3d12.h>

namespace DirectXStructToString {
	/// <summary>
	/// RootParameterやDescriptorRangeの内容を文字列に変換します。
	/// </summary>
	std::string ToString(const D3D12_ROOT_PARAMETER& rootParameter);
	std::string ToString(const D3D12_DESCRIPTOR_RANGE& descriptorRange);
}
#endif // _DEBUG
