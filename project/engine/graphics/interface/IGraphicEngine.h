#pragma once
#include "graphics/InternalApiType.h"

/// @namespace QFE::GRAPHIC
/// @brief 描画に使用する関数が定義されている名前空間
namespace QFE::GRAPHIC {
	/// @brief グラフィックエンジンのインターフェースクラス
	class IGraphicEngine {
	public:
		virtual ~IGraphicEngine() = default;

		/// @brief 内部グラフィックAPIの種類を取得する関数
		virtual QFE::GRAPHIC::InternalApiType GetInternalApiType() const = 0;

		/// @brief コアシステムが初期化済みで利用可能かを取得します。
		[[nodiscard]] virtual bool IsCoreSystemAvailable() const noexcept = 0;
		/// @brief テクスチャローダーが初期化済みで利用可能かを取得します。
		[[nodiscard]] virtual bool IsTextureLoaderAvailable() const noexcept = 0;
		/// @brief コンピュートパイプラインが初期化済みで利用可能かを取得します。
		[[nodiscard]] virtual bool IsComputePipelineAvailable() const noexcept = 0;
		/// @brief レイトレーシングパイプラインが初期化済みで利用可能かを取得します。
		[[nodiscard]] virtual bool IsRayTracingPipelineAvailable() const noexcept = 0;
		/// @brief ビルトインPSOが生成済みで利用可能かを取得します。
		[[nodiscard]] virtual bool AreBuiltInPipelineStateObjectsAvailable() const noexcept = 0;
		/// @brief 線描画機能が初期化済みで利用可能かを取得します。
		[[nodiscard]] virtual bool IsLineRenderingAvailable() const noexcept = 0;
		/// @brief ビルトインのコンピュートパイプラインステートオブジェクトが生成済みで利用可能かを取得します。
		[[nodiscard]] virtual bool IsBuildInComputePipelineStateObjectsAvailable() const noexcept = 0;
	};
}
