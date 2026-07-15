#pragma once
#include <d3d12.h>
#include <dxcapi.h>
#include <d3d12shader.h>
#include <wrl.h>
#include <vector>

#include "../pso/RootParameterElement.h"

namespace QFE::GRAPHIC {
	/// @brief シェーダーのリフレクション情報を取得するためのクラス
	class ShaderLibReflection final {
	public:
		ShaderLibReflection();
		~ShaderLibReflection() = default;

		/// @brief ルートパラメーターの要素を取得します。
		std::vector<RootParameterElement> GetRootParameterElement(IDxcBlob* shaderBlob);

	private:
		Microsoft::WRL::ComPtr<IDxcUtils> dxcUtils_;
	};
}