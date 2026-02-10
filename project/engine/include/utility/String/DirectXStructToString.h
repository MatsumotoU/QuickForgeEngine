#pragma once
#ifdef QFE_OPTIMIZE_OFF
#include <string>
#include <d3d12.h>

namespace QFE {
	namespace DirectXStructToString {
		/// @brief D3D12_ROOT_PARAMETERを文字列に変換します。
		std::string ToString(const D3D12_ROOT_PARAMETER& rootParameter);
		/// @brief D3D12_DESCRIPTOR_RANGEを文字列に変換します。
		std::string ToString(const D3D12_DESCRIPTOR_RANGE& descriptorRange);
	}
}
#endif // QFE_OPTIMIZE_OFF
