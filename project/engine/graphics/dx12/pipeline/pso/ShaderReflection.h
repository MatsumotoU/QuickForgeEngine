#pragma once
#include <d3d12.h>
#include <dxcapi.h>
#include <d3d12shader.h>
#include <wrl.h>
#include <vector>

#include "InputElement.h"
#include "RootParameterElement.h"

namespace QFE::GRAPHIC::INTERNAL {
	class ShaderReflection final {
	public:
		ShaderReflection();
		~ShaderReflection() = default;
	public:
		/// @brief シェーダーのリフレクションを実行します。shaderBlobはCompileShaderでコンパイルしたシェーダーバイナリを渡してください。
		void RunShaderReflection(IDxcBlob* shaderBlob);

		/// @brief シェーダーの入力要素を取得します。呼び出す前にRunShaderReflectionを呼び出して、shaderBlobを渡しておく必要があります。
		std::vector<InputElement> GetInputLayoutElement() const;
		/// @brief ルートパラメーターの要素を取得します。呼び出す前にRunShaderReflectionを呼び出して、shaderBlobを渡しておく必要があります。
		std::vector<RootParameterElement> GetRootParameterElement() const;

	private:
		Microsoft::WRL::ComPtr<IDxcContainerReflection> containerReflection_;
		Microsoft::WRL::ComPtr<ID3D12ShaderReflection> shaderReflection_;
	};
}
