#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxgidebug.h>
#include <dxcapi.h>
#include <map>

#include <nlohmann/json.hpp>

#include "ShaderCompiledData.h"
#include "engine/include/utility/String/MyString.h"

namespace QFE {
	class ShaderCompiler final {
	public:
		ShaderCompiler();
		~ShaderCompiler();

	public:
		/// <summary>
		/// DXCを�E期化しまぁE
		/// </summary>
		void InitializeDXC();

		/// <summary>
		/// シェーダーをコンパイルする
		/// </summary>
		/// <param name="filePath">コンパイル対象のhlslファイル吁E/param>
		/// <param name="profile">コンパイラに使用するプロファイル</param>
		/// <param name="dxUtils"></param>
		/// <param name="dxcCompiler"></param>
		/// <param name="includeHandler"></param>
		/// <returns></returns>
		IDxcBlob* CompileShader(const std::wstring& filePath, const wchar_t* profile);

		/// 指定のシェーダーのリフレクション情報をJSON形式で取得する
		nlohmann::json GetShaderReflectionJson(const std::wstring& filePath) const;
		/// 今までコンパイルしたシェーダーのリフレクション情報をJSON形式で取得する
		std::map<std::string, nlohmann::json> GetAllShaderReflectionJson() const;

	private:
		std::map<std::wstring, IDxcBlob*> iDxcBlobMap_;
		IDxcUtils* dxcUtils_;
		IDxcCompiler3* dxcCompiler_;
		IDxcIncludeHandler* includeHandler_;
	};
}