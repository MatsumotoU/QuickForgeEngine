#pragma once
#include "RootParameter.h"
#include "InputLayout.h"
#include "ShaderPair.h"
#include "RasterizerTemplate.h"
#include "BlendStates.h"
#include "DepthStencilDescTemplate.h"
#include <wrl.h>

namespace QFE::GRAPHIC::INTERNAL {
	/// @brief パイプラインステートオブジェクトを生成するための情報をまとめた構造体
	struct PipelineStateObjectElement {
		D3D12_ROOT_SIGNATURE_DESC* rootParameter;
		D3D12_INPUT_LAYOUT_DESC* inputLayoutDesc;
		D3D12_DEPTH_STENCIL_DESC depthStencilDesc;
		D3D12_PRIMITIVE_TOPOLOGY_TYPE topologyType;
		D3D12_RASTERIZER_DESC rasterizerDesc;
		D3D12_BLEND_DESC blendDesc;
		IDxcBlob* psBlob;
		IDxcBlob* vsBlob;
	};

	/// @brief パイプラインステートオブジェクトを管理するクラス
	class PipelineStateObject final {
	public:
		void CreatePipelineStateObject(
			const PipelineStateObjectElement& element, ID3D12Device* device);

		ID3D12PipelineState* GetPipelineState();
		ID3D12RootSignature* GetRootSignature();

	private:
		Microsoft::WRL::ComPtr<ID3D10Blob> signatureBlob_;
		Microsoft::WRL::ComPtr<ID3D10Blob> errorBlob_;
		Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState_;
		Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;

		bool isCreatedPipelineStateObject_ = false;
	};
}