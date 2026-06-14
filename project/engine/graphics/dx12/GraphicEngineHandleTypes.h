#pragma once
#include <cstdint>

namespace QFE::GRAPHIC {
	// * グラフィックエンジンのユーザーが扱うハンドルの定義 * //
	enum class TextureHandle : uint32_t { Invalid = UINT32_MAX };						// 画像ファイルを読み込むときに返すハンドル
	enum class ModelHandle : uint32_t { Invalid = UINT32_MAX };							// モデルデータを読み込むときに返すハンドル
	enum class VertexBufferHandle : uint32_t { Invalid = UINT32_MAX };					// 頂点データから頂点バッファを作成するときに返すハンドル
	enum class ViewPortHandle : uint32_t { Invalid = UINT32_MAX };						// ビューポートの作成時に返すハンドル
	enum class ScissorRectHandle : uint32_t { Invalid = UINT32_MAX };					// シザリング矩形の作成時に返すハンドル
	enum class VSHandle : uint32_t { Invalid = UINT32_MAX };							// 頂点シェーダーの作成時に返すハンドル
	enum class PSHandle : uint32_t { Invalid = UINT32_MAX };							// ピクセルシェーダーの作成時に返すハンドル
	enum class PSOHandle : uint32_t { Invalid = UINT32_MAX };							// パイプラインステートオブジェクトの作成時に返すハンドル
	enum class RenderTargetHandle : uint32_t { Invalid = UINT32_MAX ,SwapChain = 0};	// 描画先のリソースを作成するときに返すハンドル
	enum class DepthStencilHandle : uint32_t { Invalid = UINT32_MAX };					// 深度ステンシルのリソースを作成するときに返すハンドル

	// * ユーザーに見せない、グラフィックエンジンの実装クラスで使用するハンドルの定義 * //
	namespace INTERNAL {
		enum class DirectXResourceHandle : uint32_t { Invalid = UINT32_MAX };
		enum class TextureHandle : uint32_t { Invalid = UINT32_MAX };
		enum class ShaderPairHandle : uint32_t { Invalid = UINT32_MAX };
		enum class PSOHandle : uint32_t { Invalid = UINT32_MAX };

	}
}