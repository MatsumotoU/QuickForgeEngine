#pragma once
#include <string> 
#include <map> 

#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxgidebug.h>
#include <dxcapi.h>
#include "CompilerDevice.h"

namespace QFE::GRAPHIC::INTERNAL {
	/// @brief シェーダーをバイナリ化するクラス
	class ShaderCompiler final {
	public:
		/// @brief デバイスを初期化します
		void Initialize();
		/// @brief デバイスを解放します
		void Finalize();

		/// @brief シェーダーをコンパイルします
		IDxcBlob* CompileShader(const std::wstring& filePath, const wchar_t* profile);
		/// @brief シェーダーをキャッシュ関係なく強制的に再コンパイルします
		IDxcBlob* ForceCompileShader(const std::wstring& filePath, const wchar_t* profile);

	private:
		std::map<std::wstring, IDxcBlob*> iDxcBlobMap_;// 今までコンパイルしてきたシェーダーバイナリ
		CompilerDevice dxcDevice_;// dxcデバイス
	};
}