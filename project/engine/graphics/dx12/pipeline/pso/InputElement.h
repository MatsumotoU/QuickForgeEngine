#pragma once
#include <d3d12.h>
#include <string>

namespace QFE::GRAPHIC {
	/// @brief シェーダーの入力要素を表す構造体
	struct InputElement final {
		std::string semanticName;
		UINT semanticIndex;
		DXGI_FORMAT format;
		UINT alignedByteOffset;
	};
}
