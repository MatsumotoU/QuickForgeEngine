#pragma once
#include <functional>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxgidebug.h>
#include <dxcapi.h>
#include <wrl.h>
#include <string>
#include <memory>

#include "memory/SparseSets.h"
#include "../GraphicEngineHandleTypes.h"
#include "rtpso/RaytracingPSO.h"

namespace QFE::GRAPHIC {
	/// @brief レイトレーシングパイプラインマネージャーの初期化情報をまとめた構造体
	struct RaytracingPipelineManagerInitializeInfo {
		std::function<IDxcBlob* (const std::wstring&, const wchar_t*)> compileFunc;
		std::function<std::vector<RootParameterElement>(IDxcBlob* shaderBlob)> getRootParameterFunc;
		ID3D12Device5* device;
	};

	/// @brief レイトレーシングパイプラインの管理クラス
	class RaytracingPipelineManager final {
	public:
		RaytracingPipelineManager() = default;
		~RaytracingPipelineManager() = default;
		
		/// @brief 初期化処理
		void Initialize(const RaytracingPipelineManagerInitializeInfo& initializeInfo);
		/// @brief 終了処理
		void Finalize();

		RTPSOHandle CreateRaytracingPipelineStateObject(const std::wstring& shaderFilePath, const wchar_t* profile);

		/// @brief レイトレーシングパイプラインステートオブジェクトを取得する
		RaytracingPSO* GetRaytracingPipelineStateObject(RTPSOHandle handle);
		
	private:
		/// @brief レイトレーシングパイプラインマネージャーが有効かどうかを確認する
		bool CheckActive() const;
		/// @brief レイトレーシングシェーダーをコンパイルする
		IDxcBlob* CompileRaytracingShader(const std::wstring& filePath, const wchar_t* profile);

		bool isActive_ = false;// レイトレーシングパイプラインマネージャーが有効かどうか
		ID3D12Device5* device_ = nullptr;// レイトレーシングパイプラインマネージャーが使用するDirectX12デバイス
		std::function<IDxcBlob* (const std::wstring&, const wchar_t*)> compileFunc;// レイトレーシングシェーダーのコンパイル関数
		std::function<std::vector<RootParameterElement>(IDxcBlob* shaderBlob)> getRootParameterFunc_;// レイトレーシングシェーダーのルートパラメータを取得する関数

		SparseSet<std::unique_ptr<RaytracingPSO>> raytracingPSOs_;// レイトレーシングパイプラインステートオブジェクトの管理
	};
}