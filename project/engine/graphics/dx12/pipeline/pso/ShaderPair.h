#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxgidebug.h>
#include <dxcapi.h>
#include <string>
#include <functional>

#include "InputLayout.h"
#include "RootParameter.h"

namespace QFE::GRAPHIC {
	/// @brief シェーダーペアに使う関数群をまとめた構造体
	struct ShaderPairFunctions {
		std::function<std::vector<InputElement>(IDxcBlob* vsBlob)> getInputLayoutFunc;
		std::function<std::vector<RootParameterElement>(IDxcBlob* shaderBlob)> getRootParameterFunc;
		std::function<const D3D12_STATIC_SAMPLER_DESC* ()> getStaticSamplerFunc;
		std::function<UINT()> getStaticSamplerSizeFunc;
		std::function<UINT(IDxcBlob* shaderBlob)> getRenderTargetCountFunc;
	};

	/// @brief 頂点シェーダーとピクセルシェーダーのペアを管理するクラス
	class ShaderPair final {
	public:
		/// @brief シェーダーペアを生成します.
		void Create(IDxcBlob* vsBlob, IDxcBlob* psBlob, const ShaderPairFunctions& funcs);

		/// @brief InputLayoutを取得します
		D3D12_INPUT_LAYOUT_DESC* GetInputLayoutDesc() { return inputLayout_.GetInputLayoutDesc(); }
		/// @brief RootSignatureの説明を取得します
		D3D12_ROOT_SIGNATURE_DESC* GetRootSignatureDesc() { return rootParameter_.GetDescriptionRootSignature(); }
		/// @brief 頂点シェーダーのバイナリを取得します
		IDxcBlob* GetVSBlob() { return vsBlob_; }
		/// @brief ピクセルシェーダーのバイナリを取得します
		IDxcBlob* GetPSBlob() { return psBlob_; }
		/// @brief 登録順にRootParameterのタイプを取得します
		std::vector< D3D12_ROOT_PARAMETER_TYPE> GetRootParameterTypes() const { return rootParameter_.GetRootParameterTypes(); }
		/// @brief レンダーターゲットの数を取得します
		UINT GetRenderTargetCount() const { return renderTargetCount_; }

	private:
		bool isCreated_ = false;

		InputLayout inputLayout_;
		RootParameter rootParameter_;
		IDxcBlob* vsBlob_ = nullptr;
		IDxcBlob* psBlob_ = nullptr;
		UINT renderTargetCount_;
	};
}