#pragma once
#include <string>
#include <d3d12.h>

namespace QFE::GRAPHIC::INTERNAL {
	/// @brief ルートパラメーターの情報を管理する構造体
	struct RootParameterElement {
		std::string friendlyName;
		int shaderRegisterIndex;

		D3D_SHADER_INPUT_TYPE shaderInputType;
	};
}