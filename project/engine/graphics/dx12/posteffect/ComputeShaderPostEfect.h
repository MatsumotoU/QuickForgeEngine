#pragma once
#include "graphics/interface/IGraphicEngine.h"
#include "graphics/dx12/GraphicEngineHandleTypes.h"

namespace QFE::GRAPHIC {

	/// @brief CSを用いてポストエフェクトを行う機能
	class ComputeShaderPostEffect {
	public:
		ComputeShaderPostEffect(const ComputeShaderPostEffect&) = delete;
		ComputeShaderPostEffect& operator=(const ComputeShaderPostEffect&) = delete;
		ComputeShaderPostEffect(ComputeShaderPostEffect&&) = delete;
		ComputeShaderPostEffect& operator=(ComputeShaderPostEffect&&) = delete;

		/// @brief ビルトインのコンピュートパイプラインステートオブジェクトを生成します.
		/// @param graphicEngine 
		void CreateBuiltInComputePipelineStateObjects(IGraphicEngine* graphicEngine);

		/// @brief CSを用いてポストエフェクトを行います.
		void ApplyPostEffect();
		
	};
}