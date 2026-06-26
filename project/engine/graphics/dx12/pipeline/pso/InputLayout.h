#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxgidebug.h>

#include "InputElement.h"

#include "memory/SafeVector.h"

namespace QFE::GRAPHIC {
	/// @brief 頂点シェーダーの入力レイアウトを管理するクラス
	class InputLayout final {
	public:
		/// @brief 初期化
		void Initialize();

		/// @brief POSITION, TEXCOORD, NORMALのInputLayoutを生成します
		void CreateNormalPresetInputLayout();
		/// @brief POSITION, COLOR,TEXCOORDのInputLayoutを生成します
		void CreatePrimitivePresetInputLayout();

		/// @brief 頂点シェーダーへ送る要素を追加
		void CreateInputElementDesc(const std::string& semanticName, const UINT& semanticIndex, const DXGI_FORMAT& format, const UINT& alignedByteOffset);
		/// @brief 頂点シェーダーへ送る要素を追加
		void CreateInputElementDesc(const InputElement& inputElement);

	public:
		D3D12_INPUT_LAYOUT_DESC* GetInputLayoutDesc();

	private:
		SafeVector<std::string> semanticNames_;
		SafeVector<D3D12_INPUT_ELEMENT_DESC> inputElementDescs_;
		D3D12_INPUT_LAYOUT_DESC inputLayoutDesc_;

	};
}