#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxgidebug.h>
#include <dxcapi.h>
#include <string>
#include <functional>

#include "InputLayout.h"
#include "RootParameter.h"

namespace QFE::GRAPHIC::INTERNAL {
	/// @brief シェーダーペアに使う関数群をまとめた構造体
	struct ShaderPairFunctions {
		std::function<void(IDxcBlob* vsBlob)> reflectionFunc;
		std::function<std::vector<InputElement>(IDxcBlob* vsBlob)> getInputLayoutFunc;
		std::function<std::vector<RootParameterElement>(IDxcBlob* shaderBlob)> getRootParameterFunc;
	};

	/// @brief 頂点シェーダーとピクセルシェーダーのペアを管理するクラス
	class ShaderPair final {
	public:
		/// @brief シェーダーペアを生成します.
		void Create(IDxcBlob* vsBlob, IDxcBlob* psBlob,const ShaderPairFunctions& funcs);

	public:
		bool isCreated_ = false;

		InputLayout inputLayout_;
		RootParameter rootParameter_;
	};
}