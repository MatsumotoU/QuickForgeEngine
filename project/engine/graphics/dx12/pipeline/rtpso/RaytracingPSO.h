#pragma once
#include <wrl.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxgidebug.h>
#include <dxcapi.h>

#include "../pso/RootParameter.h"

namespace QFE::GRAPHIC {
	/// @brief レイトレーシングパイプラインの管理クラス
	class RaytracingPSO final {
	public:
		/// @brief レイトレーシングパイプラインステートオブジェクトを生成する関数
		void CreatePipelineStateObject(IDxcBlob* csBlob, // レイトレーシングシェーダーのバイナリ
			const D3D12_ROOT_SIGNATURE_DESC& rootSigDesc, ID3D12Device5* device);

		/// @brief レイトレーシングパイプラインステートオブジェクトを取得する関数
		ID3D12StateObject* GetPipelineState() const { return raytracingPipelineState_.Get(); }
		/// @brief ルートシグネチャを取得する関数
		ID3D12RootSignature* GetRootSignature() const { return rootSignature_.Get(); }
		/// @brief RootParameterを取得する関数
		RootParameter& GetRootParameter() { return rootParameter_; }

		bool CreateShaderTables(ID3D12Device5* device);

	private:
		// レイトレーシングパイプラインステートオブジェクトとルートシグネチャの管理
		Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
		Microsoft::WRL::ComPtr<ID3D12StateObject> raytracingPipelineState_;
		RootParameter rootParameter_;


		// シェーダーテーブル用のバッファ（Upload Heapで作成します）
		Microsoft::WRL::ComPtr<ID3D12Resource> rayGenShaderTable_;
		Microsoft::WRL::ComPtr<ID3D12Resource> missShaderTable_;
	};
}
