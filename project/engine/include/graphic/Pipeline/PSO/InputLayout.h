#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxgidebug.h>

#include <vector>
namespace QFE {
	class InputLayout final {
	public:
		/// <summary>
		/// 初期匁E
		/// </summary>
		void Initialize();
		/// <summary>
		/// POSITION, TEXCOORD, NORMALのInputLayoutを生成しまぁE
		/// </summary>
		void CreateNormalPresetInputLayout();
		/// <summary>
		/// 
		/// </summary>
		void CreatePrimitivePresetInputLayout();
		/// <summary>
		/// InputElementDescを生成しまぁE
		/// </summary>
		/// <param name="semanticName">ShaderInputの名前</param>
		/// <param name="semanticIndex">そ�E後に続く数孁E/param>
		/// <param name="format">format</param>
		/// <param name="alignedByteOffset">offset</param>
		void CreateInputElementDesc(const LPCSTR& semanticName, const UINT& semanticIndex, const DXGI_FORMAT& format, const UINT& alignedByteOffset);

	public:
		D3D12_INPUT_LAYOUT_DESC* GetInputLayoutDesc();

	private:
		std::vector<D3D12_INPUT_ELEMENT_DESC> inputElementDescs_;
		D3D12_INPUT_LAYOUT_DESC inputLayoutDesc_;

	};
}