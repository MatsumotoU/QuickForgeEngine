#pragma once
#include "IGraphicEngine.h"
#include "dx12/vram/descriptors/DescriptorHandles.h"
#include "dx12/GraphicEngineHandleTypes.h"
#include "dx12/pipeline/pso/PipelineDescTypes.h"

#include "dx12/DirectXDevice.h"
#include "dx12/vram/resources/DirectXResourceContainer.h"

#include "memory/UniqueContainer.h"
#include "../resources/Shaders/ShaderStructs/hlslTypeToCpp.h"

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

	/// @brief DirectX12を使用したグラフィックエンジンの実装クラス
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

		// ユーザーが任意のタイミングで呼び出す関数群
		/// @brief ビューポートの作成.
		ViewPortHandle CreateViewPort(uint32_t width, uint32_t height);
		/// @brief シザリング矩形の作成.
		ScissorRectHandle CreateScissorRect(int left, int top, int right, int bottom);
		/// @brief 画像ファイルを読み込む.
		DirectXResourceHandle CreateTextureFromFile(const std::string& filePath);
		/// @brief ビルトインテクスチャのハンドルを取得する.
		DirectXResourceHandle GetBuiltInTextureHandle(BuiltInTextureType type);

		/// @brief 頂点データから頂点バッファハンドルを作成する.
		DirectXResourceHandle CreateVertexBuffer(const std::vector<VertexData>& vertexData, const std::string& meshName);
		/// @brief シェーダーペアの作成.
		ShaderPairHandle CreateShaderPair(const ShaderPairElement& element);
		/// @brief シェーダーペアと各種情報からPSOハンドルを作成する.
		PSOHandle CreatePipelineStateObject(
			ShaderPairHandle shaderHandle, BlendMode blendMode,RasterizerType rasterizerType, DepthStencilDescType depthStencilDescType);
		PSOHandle GetBuiltInPipelineStateObject(
			BuiltInShaderPair builtInShaderPair, BlendMode blendMode, RasterizerType rasterizerType, DepthStencilDescType depthStencilDescType);
		
		/// @brief コンピュートシェーダーのPSOを生成します
		ComputePSOHandle CreateComputePipelineStateObject(const std::string& dirPath, const std::string& csFileName);

		/// @brief あるリソースの配列の数を取得する.
		size_t GetResourceArraySize(DirectXResourceHandle handle);

		/// @brief 定数バッファを作成し、データをコピーする.データの型はテンプレートで指定する.
		template<typename T>
		DirectXResourceHandle CreateConstantBuffer(const T& data, const std::string& bufferName) {
			DirectXResourceHandle handle = resourceContainer_->CreateBuffer(directXDevice_->GetDevice(), sizeof(T));
			resourceContainer_->MapResource(handle);
			T* mappedData = resourceContainer_->template GetMappedData<T>(handle);
			if (mappedData) {
				memcpy(mappedData, &data, sizeof(T));
			}
			resourceContainer_->SetResourceStrideInBytes(handle, sizeof(T));
			return handle;
		}
		/// @brief 定数バッファのデータを取得する.データの型はテンプレートで指定する.
		template<typename T>
		T* GetConstantBufferData(DirectXResourceHandle handle) {
			return resourceContainer_->template GetMappedData<T>(handle);
		}

		/// @brief UAVバッファを作成
		DirectXResourceHandle CreateUAVBuffer(uint32_t width, uint32_t height);

		RTPSOHandle CreateRayTracingPipelineStateObject(const std::string& dirPath, const std::string& rgsFileName);
		
		void TestDraw(
			PSOHandle psoHandle,ViewPortHandle viewportHandle, ScissorRectHandle scissorRectHandle,
			DirectXResourceHandle vertexBufferHandle,std::vector<DirectXResourceHandle> rootResources);

		void TestCompute(
			ComputePSOHandle computePSOHandle, DirectXResourceHandle uavHandle, DirectXResourceHandle constantBufferHandle);

	private:
		/// @brief DirectXCommonの名残.fenceの初期化以降の処理.
		void LegacyInitialize(uint32_t width, uint32_t height);
		DirectXResourceHandle CreateDepthStencilBuffer(uint32_t width, uint32_t height);
		void ClearDepthStencil(DirectXResourceHandle depthStencilHandle);

		HWND hwnd_;// ウィンドウハンドル
		std::unique_ptr<DirectX12DebugCore> debugCore_;// DirectX12のデバッグコアクラス

		std::unique_ptr<DirectXDevice> directXDevice_;// DirectX12の共通管理クラス
		std::unique_ptr<DirectXResourceContainer> resourceContainer_;// DirectX12のリソース管理クラス
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
	};
}