#pragma once
namespace QFE::FRAMEWORK {
	/// @brief グラフィックエンジンで利用可否を確認できる機能の種類
	enum class GraphicEngineFeature {
		CoreSystem,
		TextureLoader,
		ComputePipeline,
		RayTracingPipeline,
		BuiltInPipelineStateObjects,
		LineRendering
	};

	/// @brief グラフィックエンジンの機能が有効かどうかを表す構造体
	struct GraphicEngineFeatureState final {
		bool isRaytracingEnabled = false; ///< レイトレーシング機能が有効かどうか
		bool isComputeEnabled = false; ///< コンピュート機能が有効かどうか
		bool isTextureLoaderEnabled = false; ///< テクスチャローダー機能が有効かどうか
		bool isLineRenderingEnabled = false; ///< 線描画機能が有効かどうか
		bool isBuildInPipelineStateObjectsEnabled = false; ///< ビルトインパイプラインステートオブジェクトが有効かどうか
	};
}
