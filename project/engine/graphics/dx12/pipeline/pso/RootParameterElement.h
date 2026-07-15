#pragma once
#include <string>
#include <d3d12.h>

namespace QFE::GRAPHIC {
	/// @brief ルートパラメーターの情報を管理する構造体
	struct RootParameterElement {
		std::string friendlyName;// ルートパラメータのフレンドリ名を保持するメンバ変数

		int shaderRegisterIndex;// ルートパラメータのシェーダーレジスタインデックスを保持するメンバ変数
		D3D_SHADER_INPUT_TYPE shaderInputType;// ルートパラメータのシェーダー入力タイプを保持するメンバ変数
	};
}