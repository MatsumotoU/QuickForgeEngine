#pragma once
#include <d3d12.h>
#include <dxcapi.h>
#include <d3d12shader.h>
#include <wrl.h>
#include <vector>

#include "InputElement.h"
#include "RootParameterElement.h"

namespace QFE::GRAPHIC {
	class ShaderReflection final {
	public:
		ShaderReflection();
		~ShaderReflection() = default;
	public:
		/// @brief シェーダーのリフレクションを実行します。shaderBlobはCompileShaderでコンパイルしたシェーダーバイナリを渡してください。
		void RunShaderReflection(IDxcBlob* shaderBlob);

		/// @brief シェーダーの入力要素を取得します。shaderBlobはCompileShaderでコンパイルしたシェーダーバイナリを渡してください。
		std::vector<InputElement> GetInputLayoutElement(IDxcBlob* shaderBlob);
		/// @brief ルートパラメーターの要素を取得します。shaderBlobはCompileShaderでコンパイルしたシェーダーバイナリを渡してください。
		std::vector<RootParameterElement> GetRootParameterElement(IDxcBlob* shaderBlob);
		/// @brief シェーダーのスレッドグループサイズを取得します。shaderBlobはCompileShaderでコンパイルしたシェーダーバイナリを渡してください。
		bool GetThreadGroupSize(IDxcBlob* shaderBlob, UINT& sizeX, UINT& sizeY, UINT& sizeZ);

		/// @brief シェーダーの入力要素を取得します。呼び出す前にRunShaderReflectionを呼び出して、shaderBlobを渡しておく必要があります。
		std::vector<InputElement> GetInputLayoutElement() const;
		/// @brief ルートパラメーターの要素を取得します。呼び出す前にRunShaderReflectionを呼び出して、shaderBlobを渡しておく必要があります。
		std::vector<RootParameterElement> GetRootParameterElement() const;
		/// @brief シェーダーのスレッドグループサイズを取得します。呼び出す前にRunShaderReflectionを呼び出して、shaderBlobを渡しておく必要があります。
		bool GetThreadGroupSize(UINT& sizeX, UINT& sizeY, UINT& sizeZ) const;

	private:
		Microsoft::WRL::ComPtr<IDxcContainerReflection> containerReflection_;
		Microsoft::WRL::ComPtr<ID3D12ShaderReflection> shaderReflection_;
	};
}
