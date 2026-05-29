/**
 * @file GraphicPipelineManager.h
 * @brief グラフィックスパイプライン(PSO)を管理するクラス
 */

#pragma once
#include "../Descriptors/DsvDescriptorHeap.h"

#include "pso/PipelineStateObject.h"
#include "pso/ShaderReflection.h"

namespace QFE {
	/**
	 * @class GraphicPipelineManager
	 * @brief 各種ブレンドモードやシェーダーに対応したPSOを一括管理するシングルトンクラス
	 */
	class GraphicPipelineManager final {
	public:
		/** @brief 初期化処理 */
		void Initialize(ID3D12Device* device,const D3D12_DEPTH_STENCIL_DESC& depthStencilDesc);

		/** @brief 三角形描画用のPSOを取得 */
		PipelineStateObject* GetTrianglePso(BlendMode blendmode) { return &trianglePso_[static_cast<uint32_t>(blendmode)]; }
		/** @brief スプライト描画用のPSOを取得 */
		PipelineStateObject* GetSpritePso(BlendMode blendmode) { return &spritePso_[static_cast<uint32_t>(blendmode)]; }
		/** @brief ライン描画用のPSOを取得 */
		PipelineStateObject* GetLinePso(BlendMode blendmode) { return &linePso_[static_cast<uint32_t>(blendmode)]; }
		/** @brief 点描画用のPSOを取得 */
		PipelineStateObject* GetPointPso(BlendMode blendmode) { return &pointPso_[static_cast<uint32_t>(blendmode)]; }
		/** @brief プリミティブ描画用のPSOを取得 */
		PipelineStateObject* GetPrimitivePso(BlendMode blendmode) { return &primitivePso_[static_cast<uint32_t>(blendmode)]; }
		/** @brief パーティクル描画用のPSOを取得 */
		PipelineStateObject* GetParticlePso(BlendMode blendmode) { return &particlePso_[static_cast<uint32_t>(blendmode)]; }
		/** @brief 色補正用のPSOを取得 */
		PipelineStateObject* GetColorCorrectionPso() { return &colorCorrectionPso_; }
		/** @brief グレースケール用のPSOを取得 */
		PipelineStateObject* GetGrayScalePso() { return &grayScaleTrianglePso_; }
		/** @brief ビネット用のPSOを取得 */
		PipelineStateObject* GetVignettePso() { return &vignettePso_; }
		/** @brief 法線表示用のPSOを取得 */
		PipelineStateObject* GetNormalPso() { return &normalPso_; }
		/** @brief フォント描画用のPSOを取得 */
		PipelineStateObject* GetFontPso() { return &fontPso_; }
		/** @brief ピクセル化用のPSOを取得 */
		PipelineStateObject* GetPixelPso() { return &pixelPso_; }
		/** @brief ボックスフィルタ用のPSOを取得 */
		PipelineStateObject* GetBoxFilterPso() { return &boxFilterPso_; }

		/// @brief スカイボックス描画用のPSOを取得
		PipelineStateObject* GetSkyBoxPso() { return &skyBoxPso_; }

	private: // メンバ変数
		ShaderCompiler shaderCompiler_;

		// InputLayout
		InputLayout normalInputLayout_;
		InputLayout primitiveInputLayout_;

		// ルートパラメータ
		std::map<std::string, RootParameter> rootParameterMap_;
		RootParameter normalGameObjectRootParameter_;
		RootParameter spriteObjectRootParameter_;
		RootParameter particleRootParameter_;
		RootParameter primitiveRootParameter_;
		RootParameter grayScaleRootParameter_;
		RootParameter boxFilterRootParameter_;
		RootParameter colorCorrectionRootParameter_;
		RootParameter vignetteRootParameter_;
		RootParameter normalRootParameter_;
		RootParameter fontRootParameter_;
		RootParameter pixelRootParameter_;
		RootParameter skyBoxRootParameter_;

		// PSO
		PipelineStateObject trianglePso_[kCountOfBlendMode];
		PipelineStateObject linePso_[kCountOfBlendMode];
		PipelineStateObject pointPso_[kCountOfBlendMode];

		PipelineStateObject spritePso_[kCountOfBlendMode];

		PipelineStateObject primitivePso_[kCountOfBlendMode];

		PipelineStateObject particlePso_[kCountOfBlendMode];

		PipelineStateObject boxFilterPso_;
		PipelineStateObject colorCorrectionPso_;
		PipelineStateObject grayScaleTrianglePso_;
		PipelineStateObject vignettePso_;
		PipelineStateObject pixelPso_;

		PipelineStateObject fontPso_;

		PipelineStateObject normalPso_;

		PipelineStateObject skyBoxPso_;
	};
}