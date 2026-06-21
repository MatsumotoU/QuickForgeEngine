#pragma once
#include <d3d12.h>

namespace QFE::GRAPHIC {
	/// @brief シェーダーの入力要素を表す構造体
	struct InputElement final {
		LPCSTR semanticName;
		UINT semanticIndex;
		DXGI_FORMAT format;
		UINT alignedByteOffset;
	};
}
