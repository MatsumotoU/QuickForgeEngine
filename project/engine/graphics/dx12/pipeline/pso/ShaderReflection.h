#pragma once
#include <d3d12.h>
#include <dxcapi.h>
#include <d3d12shader.h>
#include <wrl.h>
#include <vector>

#include "InputElement.h"
#include "RootParameterElement.h"

namespace QFE::GRAPHIC {
	/// @brief VS,PS, CSなどのシェーダーのリフレクション情報を取得するクラス
	class ShaderReflection final {
	public:
		/// @brief シェーダーの入力要素を取得します。shaderBlobはCompileShaderでコンパイルしたシェーダーバイナリを渡してください。
		std::vector<InputElement> GetInputLayoutElement(IDxcBlob* shaderBlob);
		/// @brief ルートパラメーターの要素を取得します。shaderBlobはCompileShaderでコンパイルしたシェーダーバイナリを渡してください。
		std::vector<RootParameterElement> GetRootParameterElement(IDxcBlob* shaderBlob);
		/// @brief シェーダーのスレッドグループサイズを取得します。shaderBlobはCompileShaderでコンパイルしたシェーダーバイナリを渡してください。
		bool GetThreadGroupSize(IDxcBlob* shaderBlob, UINT& sizeX, UINT& sizeY, UINT& sizeZ);

	private:
		/// @brief IDxcContainerReflectionのインスタンスを作成します。
		Microsoft::WRL::ComPtr<IDxcContainerReflection> CreateContainerReflection(IDxcBlob* shaderBlob);
		/// @brief ID3D12ShaderReflectionのインスタンスを作成します。
		Microsoft::WRL::ComPtr<ID3D12ShaderReflection> CreateShaderReflection(IDxcBlob* shaderBlob);
	};
}
