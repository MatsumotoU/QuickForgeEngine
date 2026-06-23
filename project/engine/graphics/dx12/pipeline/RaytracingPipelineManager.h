#pragma once
#include <functional>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxgidebug.h>
#include <dxcapi.h>
#include <wrl.h>
#include <string>

#include "../GraphicEngineHandleTypes.h"

namespace QFE::GRAPHIC {
	/// @brief レイトレーシングパイプラインマネージャーの初期化情報をまとめた構造体
	struct RaytracingPipelineManagerInitializeInfo {
		std::function<IDxcBlob* (const std::wstring&, const wchar_t*)> compileFunc;
		ID3D12Device* device;
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

		void CompileRaytracingShader(const std::wstring& filePath, const wchar_t* profile);

		
	private:
		bool isActive_ = false;
		Microsoft::WRL::ComPtr<ID3D12Device5> device_;
		std::function<IDxcBlob* (const std::wstring&, const wchar_t*)> compileFunc;
	};
}