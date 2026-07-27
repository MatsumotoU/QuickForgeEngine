#pragma once
#include "IGraphicEngine.h"
#include "dx12/vram/descriptors/DescriptorHandles.h"
#include "dx12/GraphicEngineHandleTypes.h"
#include "dx12/pipeline/pso/PipelineDescTypes.h"

#include "dx12/DirectXDevice.h"
#include "dx12/vram/resources/DirectXResourceContainer.h"
#include "dx12/vram/DirectXResourceAllocator.h"

#include "memory/UniqueContainer.h"
#include "../resources/Shaders/ShaderStructs/hlslTypeToCpp.h"

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
	class D3D12GraphicEngine final : public IGraphicEngine {
	public:
		/// @brief wndowsに依存したグラフィックエンジンです.描画ウィンドウのハンドルの引数に取ります.
		explicit D3D12GraphicEngine(HWND hwnd);
		~D3D12GraphicEngine() override;

		// 一度は呼ぶ順番がある関数群
		void Initialize() override;
		void PreDraw() override;
		void Draw() override {};
		void PostDraw() override;
		void Shutdown() override;

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

		DirectXResourceHandle depthStencilBufferHandle_;// 深度ステンシルバッファのリソースハンドル

		RaytracingAccelerationStructure accelerationStructure_; // レイトレーシング用の加速構造の管理クラス
	};
}