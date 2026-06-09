#pragma once
#include <functional>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxgidebug.h>
#include <dxcapi.h>
#include <string>

#include "../Descriptors/DsvDescriptorHeap.h"

#include "pso/PipelineStateObject.h"
#include "pso/ShaderReflection.h"

namespace QFE::GRAPHIC::INTERNAL {

	/// @brief グラフィックスパイプラインおよびルートシグネチャの管理クラス
	class GraphicPipelineManager final {
	public:
		/// @brief 初期化処理
		void Initialize(std::function<IDxcBlob*(const std::wstring&, const wchar_t*)> compileFunc);
		/// @brief 終了処理
		void Finalize();

	private: // メンバ変数
	};
}