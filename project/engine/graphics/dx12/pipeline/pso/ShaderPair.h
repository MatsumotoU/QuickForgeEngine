#pragma once
#include <string>
#include <memory>
#include <d3d12.h>

namespace QFE::GRAPHIC::INTERNAL {
	/// @brief 頂点シェーダーとピクセルシェーダーのペアを表すクラス
	class ShaderPair final {
	public:
		/// @brief 頂点シェーダーとピクセルシェーダーのペアを作成する
		void Create(ID3DBlob* vsBlob, ID3DBlob* psBlob);

	public:
	};
}