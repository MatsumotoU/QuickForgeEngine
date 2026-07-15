#pragma once
#include "memory/SparseSets.h"
#include "../GraphicEngineHandleTypes.h"
#include "pso/RootParameterElement.h"
#include "compute/ComputePSO.h"

#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxgidebug.h>
#include <dxcapi.h>

#include <functional>
#include <string>
#include <memory>
#include <map>

namespace QFE::GRAPHIC {
	/// @brief コンピュートパイプラインマネージャーの初期化情報をまとめた構造体
	struct ComputePipelineManagerInitializeInfo {
		std::function<std::vector<RootParameterElement>(IDxcBlob* shaderBlob)> getRootParameterFunc;
		std::function<bool(IDxcBlob* shaderBlob, UINT& sizeX, UINT& sizeY, UINT& sizeZ)> getThreadGroupSizeFunc;
		std::function<IDxcBlob* (const std::wstring&, const wchar_t*)> compileFunc;
		ID3D12Device* device;
	};

	/// @brief コンピュートパイプラインを管理するクラス
	class ComputePipelineManager final {
	public:
		ComputePipelineManager();

		/// @brief 初期化処理
		void Initialize(ComputePipelineManagerInitializeInfo initializeInfo);

		/// @brief コンピュートパイプラインステートオブジェクトを生成します
		ComputePSOHandle GenerateComputePipelineStateObject(
			const std::string& csDirPath, const std::string& csFileName);

		/// @brief コンピュートパイプラインステートオブジェクトを取得します
		ID3D12RootSignature* GetRootSignature(const ComputePSOHandle& handle) const;
		/// @brief コンピュートパイプラインステートオブジェクトを取得します
		ID3D12PipelineState* GetPipelineState(const ComputePSOHandle& handle) const;
		/// @brief コンピュートパイプラインステートオブジェクトのルートパラメータタイプを取得します
		std::vector<D3D12_ROOT_PARAMETER_TYPE> GetRootParameterTypes(const ComputePSOHandle& handle) const;
		/// @brief コンピュートパイプラインステートオブジェクトのスレッドグループサイズを取得します
		bool GetThreadGroupSize(const ComputePSOHandle& handle, UINT& sizeX, UINT& sizeY, UINT& sizeZ) const;

	private:
		ComputePipelineManagerInitializeInfo initializeInfo_;// 初期化情報を保持
		SparseSet<std::unique_ptr<ComputePSO>> computePSOs_;// コンピュートパイプラインステートオブジェクトの管理
		std::map<std::string, IDxcBlob*> csBlobMap_;// コンピュートシェーダーのバイナリを管理するマップ
	};
}