#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxgidebug.h>
#include <dxcapi.h>
#include <string>

#include "InputLayout.h"

namespace QFE::GRAPHIC::INTERNAL {
	class ShaderPair final {
	public:
		/// @brief シェーダーペアを生成します.
		void Create(IDxcBlob* vsBlob, IDxcBlob* psBlob);

	public:
		bool isCreated_ = false;

		InputLayout inputLayout_;
	};
}