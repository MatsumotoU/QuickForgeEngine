#pragma once
#include "IGraphicEngine.h"
#include "dx12/vram/descriptors/DescriptorHandles.h"
#include "dx12/GraphicEngineHandleTypes.h"
#include "dx12/pipeline/pso/PipelineDescTypes.h"

#include "dx12/DirectXDevice.h"
#include "dx12/vram/resources/DirectXResourceContainer.h"
#include "dx12/vram/DirectXResourceAllocator.h"

#include "memory/UniqueContainer.h"
#include "resources/shaders/shaderStructs/hlslTypeToCpp.h"

#include "dx12/pipeline/rtpso/RaytracingAccelerationStructure.h"

#include "string/MyString.h"

#define NOMINMAX
#include <windows.h>
#include <d3d12.h>
#include <wrl.h>
#include <stdint.h>
#include <string>
#include <utility>
#include <vector>
#include <memory>
#include <cstddef>

namespace QFE::GRAPHIC {
	/// @brief DirectX12のグラフィックエンジンの実装クラスで使用しているクラス
	class DirectX12DebugCore;
	//class DirectXDevice;
	//class DirectXResourceContainer;
	class DescriptorHeapManager;
	class DirectXCommandManager;
	class Fence;
	class ShaderReflection;
	class ShaderLibReflection;
	class ShaderCompiler;
	class GraphicPipelineManager;
	class ComputePipelineManager;
	class RaytracingPipelineManager;
	class TextureLoader;
	class RenderPass;

	/// @brief ビルトインテクスチャの種類を表す列挙型
	enum class BuiltInTextureType {
		DummyBlackCubeMap,
		DummyWhite1x1Texture
	};

	/// @brief DirectX12を使用したグラフィック機能のラッパー所持クラス
	class D3D12GraphicEngine : public IGraphicEngine {
	public:
		/// @brief wndowsに依存したグラフィックエンジンです.描画ウィンドウのハンドルの引数に取ります.
		explicit D3D12GraphicEngine(HWND hwnd);
		~D3D12GraphicEngine() override;
		D3D12GraphicEngine(const D3D12GraphicEngine&) = delete;
		D3D12GraphicEngine& operator=(const D3D12GraphicEngine&) = delete;
		D3D12GraphicEngine(D3D12GraphicEngine&&) = delete;
		D3D12GraphicEngine& operator=(D3D12GraphicEngine&&) = delete;

		/// @brief どんな機能を使うにも必要な機能をたちあげます.
		bool SetUpCoreSystem();
		/// @brief テクスチャローダーをたちあげます.
		bool SetUpTextureLoader();
		/// @brief コンピュートパイプラインをたちあげます.
		bool SetUpComputePipeline();
		/// @brief レイトレーシングパイプラインをたちあげます.
		bool SetUpRayTracingPipeline();
		/// @brief ビルトインのパイプラインステートオブジェクトを生成します
		bool SetUpBuildInPipelineStateObjects();
		/// @brief 線描画用のパイプラインをたちあげます.
		bool SetUpLineRenderingPipeline();

		/// @brief 描画の前処理を行います.描画の前に呼び出してください.
		void PreDraw();
		/// @brief 描画の後処理を行います.描画の後に呼び出してください.
		void PostDraw();
		/// @brief 終了処理を行います.グラフィックエンジンの終了時に呼び出してください.
		void Shutdown();

		/// @brief サポートしている内部グラフィックAPIの種類を取得します。
		QFE::GRAPHIC::InternalApiType GetInternalApiType() const override;
		[[nodiscard]] bool IsCoreSystemAvailable() const noexcept override;
		[[nodiscard]] bool IsTextureLoaderAvailable() const noexcept override;
		[[nodiscard]] bool IsComputePipelineAvailable() const noexcept override;
		[[nodiscard]] bool IsRayTracingPipelineAvailable() const noexcept override;
		[[nodiscard]] bool AreBuiltInPipelineStateObjectsAvailable() const noexcept override;
		[[nodiscard]] bool IsLineRenderingAvailable() const noexcept override;

		/// @brief 次のフレームに描画するワールド空間の線を追加します。
		void DrawLine(
			const QFE::MATH::Vector3& start,
			const QFE::MATH::Vector3& end,
			const QFE::MATH::Vector4& color = { 1.0f, 1.0f, 1.0f, 1.0f });
		/// @brief キューに追加された線を現在のフレームに描画します。
		void RenderLines(
			const QFE::MATH::Matrix4x4& viewProjection,
			ViewPortHandle viewportHandle,
			ScissorRectHandle scissorRectHandle,
			RenderTargetHandle renderTargetHandle);

		/// @brief 定数バッファのデータを取得する.データの型はテンプレートで指定する.
		template<typename T>
		T* GetConstantBufferData(DirectXResourceHandle handle) {
			return resourceContainer_->template GetMappedData<T>(handle);
		}

		// * 各機能の取得 * //
		/// @brief DirectX12のデバイスラップクラスを取得する
		DirectXDevice* GetDirectXDevice() const;
		/// @brief DirectX12のリソースコンテナクラスを取得する
		DirectXResourceContainer* GetDirectXResourceContainer() const;
		/// @brief DirectX12のリソース割り当てクラスを取得する
		DirectXResourceAllocator* GetDirectXResourceAllocator() const;
		/// @brief DirectX12のデスクリプタヒープ管理クラスを取得する
		DescriptorHeapManager* GetDescriptorHeapManager() const;
		/// @brief DirectX12のコマンド管理クラスを取得する
		DirectXCommandManager* GetDirectXCommandManager() const;
		/// @brief DirectX12のフェンス管理クラスを取得する
		Fence* GetFence() const;
		/// @brief DirectX12のシェーダーリフレクションクラスを取得する
		ShaderReflection* GetShaderReflection() const;
		/// @brief DirectX12のシェーダーライブラリリフレクションクラスを取得する
		ShaderLibReflection* GetShaderLibReflection() const;
		/// @brief DirectX12のシェーダーコンパイルクラスを取得する
		ShaderCompiler* GetShaderCompiler() const;
		/// @brief DirectX12のグラフィックパイプライン管理クラスを取得する
		GraphicPipelineManager* GetGraphicPipelineManager() const;
		/// @brief DirectX12のコンピュートパイプライン管理クラスを取得する
		ComputePipelineManager* GetComputePipelineManager() const;
		/// @brief DirectX12のレイトレーシングパイプライン管理クラスを取得する
		RaytracingPipelineManager* GetRayTracingPipelineManager() const;
		/// @brief DirectX12のテクスチャ管理クラスを取得する
		TextureLoader* GetTextureLoader() const;
		/// @brief DirectX12のビューポートのコンテナを取得する
		QFE::UniqueContainer<D3D12_VIEWPORT>& GetViewports();
		/// @brief DirectX12のシザリング矩形のコンテナを取得する
		QFE::UniqueContainer<D3D12_RECT>& GetScissorRects();
		/// @brief DirectX12のレンダーターゲット管理クラスを取得する
		RenderPass* GetRenderPass() const;
		/// @brief DirectX12のレイトレーシング用の加速構造の管理クラスを取得する
		RaytracingAccelerationStructure* GetRaytracingAccelerationStructure();

		/// @brief DirectX12の深度ステンシルバッファのリソースハンドルを取得する
		DirectXResourceHandle GetDepthStencilBufferHandle() const;

	private:
		/// @brief DirectXCommonの名残.fenceの初期化以降の処理.
		void LegacyInitialize(uint32_t width, uint32_t height);
		DirectXResourceHandle CreateDepthStencilBuffer(uint32_t width, uint32_t height);
		void ClearDepthStencil(DirectXResourceHandle depthStencilHandle);

		HWND hwnd_;// ウィンドウハンドル
		std::unique_ptr<DirectX12DebugCore> debugCore_;// DirectX12のデバッグコアクラス

		std::unique_ptr<DirectXDevice> directXDevice_;// DirectX12の共通管理クラス
		std::unique_ptr<DirectXResourceContainer> resourceContainer_;// DirectX12のリソース管理クラス
		std::unique_ptr<DirectXResourceAllocator> resourceAllocator_;// DirectX12のリソース割り当てクラス
		std::unique_ptr<DescriptorHeapManager> descriptorHeapManager_;// デスクリプタヒープ管理クラス
		std::unique_ptr<DirectXCommandManager> commandManager_;// コマンド管理クラス
		std::unique_ptr<RenderPass> renderPass_;// 描画先管理クラス
		std::unique_ptr<Fence> fence_;// フェンス管理クラス

		std::unique_ptr<ShaderReflection> shaderReflection_;// シェーダーリフレクションクラス
		std::unique_ptr<ShaderLibReflection> shaderLibReflection_;// シェーダーライブラリリフレクションクラス
		std::unique_ptr<ShaderCompiler> shaderCompiler_;// シェーダーコンパイルクラス
		std::unique_ptr<GraphicPipelineManager> graphicPipelineManager_;// グラフィックパイプライン管理クラス
		std::unique_ptr<ComputePipelineManager> computePipelineManager_;// コンピュートパイプライン管理クラス
		std::unique_ptr<RaytracingPipelineManager> rayTracingPipelineManager_;// レイトレーシングパイプライン管理クラス

		std::unique_ptr<TextureLoader> textureLoader_;// テクスチャ管理クラス

		QFE::UniqueContainer<D3D12_VIEWPORT> viewports_;// ビューポートのコンテナ
		QFE::UniqueContainer<D3D12_RECT> scissorRects_;// シザリング矩形のコンテナ

		DirectXResourceHandle depthStencilBufferHandle_ = DirectXResourceHandle::Invalid;// 深度ステンシルバッファのリソースハンドル

		std::unique_ptr<RaytracingAccelerationStructure> accelerationStructure_; // レイトレーシング用の加速構造の管理クラス

		bool isCoreSystemAvailable_ = false;
		bool isTextureLoaderAvailable_ = false;
		bool isComputePipelineAvailable_ = false;
		bool isRayTracingPipelineAvailable_ = false;
		bool areBuiltInPipelineStateObjectsAvailable_ = false;
		bool isLineRenderingAvailable_ = false;

		struct LineVertex {
			QFE::MATH::Vector4 position;
			QFE::MATH::Vector4 color;
		};
		static constexpr std::size_t kMaxLineCount_ = 1024;
		std::vector<LineVertex> lineVertices_;
		DirectXResourceHandle lineVertexBufferHandle_ = DirectXResourceHandle::Invalid;
		PSOHandle linePsoHdrHandle_ = PSOHandle::Invalid;
		PSOHandle linePsoSrgbHandle_ = PSOHandle::Invalid;
		PSOHandle linePsoUnormHandle_ = PSOHandle::Invalid;
	};
}
