#pragma once
#include <wrl.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxgidebug.h>
#include <dxcapi.h>

#include "../pso/RootParameter.h"

namespace QFE::GRAPHIC {
	/// @brief コンピュートパイプラインの管理クラス
	class ComputePSO final {
	public:
		/// @brief コンピュートパイプラインステートオブジェクトを生成する関数
		void CreatePipelineStateObject(IDxcBlob* csBlob, // コンピュートシェーダーのバイナリ
			const D3D12_ROOT_SIGNATURE_DESC& rootSigDesc,ID3D12Device* device);

		/// @brief コンピュートパイプラインステートオブジェクトを取得する関数
		ID3D12PipelineState* GetPipelineState() const { return computePipelineState_.Get(); }
		/// @brief ルートシグネチャを取得する関数
		ID3D12RootSignature* GetRootSignature() const { return rootSignature_.Get(); }
		/// @brief RootParameterを取得する関数
		RootParameter& GetRootParameter() { return rootParameter_; }
		
	private:
		Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
		Microsoft::WRL::ComPtr<ID3D12PipelineState> computePipelineState_;
		RootParameter rootParameter_;
	};
}
