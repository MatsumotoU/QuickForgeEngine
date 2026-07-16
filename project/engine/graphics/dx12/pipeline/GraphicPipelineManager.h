#pragma once
#include <functional>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxgidebug.h>
#include <dxcapi.h>
#include <string>

#include <map>
#include <memory>
#include <vector>

#include "memory/MultidimensionalArray.h"
#include "memory/SparseSets.h"

#include "../GraphicEngineHandleTypes.h"
#include "pso/PipelineDescTypes.h"
#include "pso/InputElement.h"
#include "pso/RootParameterElement.h"

namespace QFE::GRAPHIC {
	/// @brief グラフィックスパイプラインを作るためのクラス群の前方宣言
	class ShaderReflection;
	class StaticSamplerTemplate;
	class ShaderPair;
	class RasterizerTemplate;
	class BlendStateTemplate;
	class DepthStencilDescTemplate;
	class PipelineStateObject;

	/// @brief グラフィックスパイプラインマネージャーの初期化情報をまとめた構造体
	struct GraphicPipelineManagerInitializeInfo {
		std::function<std::vector<InputElement>(IDxcBlob* shaderBlob)> getInputLayoutFunc;
		std::function<std::vector<RootParameterElement>(IDxcBlob* shaderBlob)> getRootParameterFunc;
		std::function<UINT(IDxcBlob* shaderBlob)> getRenderTargetCountFunc;
		std::function<IDxcBlob*(const std::wstring&, const wchar_t*)> compileFunc;
		ID3D12Device* device;
	};

	/// @brief グラフィックスパイプラインおよびルートシグネチャの管理クラス
	class GraphicPipelineManager final {
	public:
		explicit GraphicPipelineManager();
		~GraphicPipelineManager();

	public:
		/// @brief 初期化処理
		void Initialize(GraphicPipelineManagerInitializeInfo initializeInfo);
		/// @brief 終了処理
		void Finalize();

		/// @brief シェーダーペアを生成します
		ShaderPairHandle GenerateShaderPair(
			const ShaderPairElement& element);
		/// @brief パイプラインステートオブジェクトを生成します
		PSOHandle GeneratePipelineStateObject(
			const ShaderPairHandle& shaderHandle, ID3D12Device* device
			, D3D12_PRIMITIVE_TOPOLOGY_TYPE topologyType, D3D12_RASTERIZER_DESC rasterizerDesc,
			D3D12_BLEND_DESC blendDesc, D3D12_DEPTH_STENCIL_DESC depthStencilDesc);
		/// @brief シェーダーペアと各種情報からパイプラインステートオブジェクトを生成します
		PSOHandle GeneratePipelineStateObject(
			ID3D12Device* device, const ShaderPairHandle& shaderHandle, BlendMode blendMode,
			RasterizerType rasterizerType, DepthStencilDescType depthStencilDescType);

		/// @brief PSOハンドルからパイプラインステートオブジェクトを取得します
		PipelineStateObject* GetPipelineStateObject(const PSOHandle& psoHandle) const;
		/// @brief PSOハンドルからパイプラインステートを取得します
		ID3D12PipelineState* GetPipelineState(const PSOHandle& psoHandle) const;
		/// @brief PSOハンドルからルートシグネチャを取得します
		ID3D12RootSignature* GetRootSignature(const PSOHandle& psoHandle) const;
		/// @brief PSOハンドルからルートパラメータのタイプを取得します
		std::vector<D3D12_ROOT_PARAMETER_TYPE> GetRootParameterTypes(const PSOHandle& psoHandle) const;
		/// @brief BuiltInのシェーダーペアと各情報からPSOハンドルを取得します
		PSOHandle GetBuiltInPSOHandle(
			BuiltInShaderPair builtInShaderPair, BlendMode blendMode,
			RasterizerType rasterizerType, DepthStencilDescType depthStencilDescType) const;
		
	private: // メンバ変数
		/// @brief BuiltInのシェーダーペアを生成します
		void GenerateBuiltInShaderPairs();
		/// @brief BuiltInのPSOを各情報すべてから生成します
		void GenerateBuiltInPSO(ID3D12Device* device);

		GraphicPipelineManagerInitializeInfo initializeInfo_;// 初期化情報を保持する構造体

		// グラフィックスパイプラインを管理するためのメンバ変数
		uint32_t shaderPairKeyCounter_ = 0;// シェーダーペアのキーを管理するカウンター
		std::map<std::string, uint32_t> shaderPairNameToKeyMap_;// シェーダーペアの名前からキーを取得するためのマップ
		std::map<uint32_t, std::unique_ptr<ShaderPair>> shaderPairs_;// シェーダーペアのマップ
		uint32_t pipelineStateObjectKeyCounter_ = 0;// パイプラインステートオブジェクトのキーを管理するカウンター
		std::map<std::string, uint32_t> pipelineStateObjectNameToKeyMap_;// パイプラインステートオブジェクトの名前からキーを取得するためのマップ
		std::map<uint32_t, std::unique_ptr<PipelineStateObject>> pipelineStateObjects_;// パイプラインステートオブジェクトのマップ

		std::map<BuiltInShaderPair, ShaderPairHandle> builtInPairHandles_;// BuiltInのPSOのハンドルを管理するマップ
		// pairHandle,blend,rasterize,depthの順番
		MultidimensionalArray<PSOHandle, 4> BuiltInPSOs_;// ビルトインのPSOを管理する多次元配列

		std::unique_ptr<StaticSamplerTemplate> staticSamplers_;// 静的サンプラーの管理クラス
		std::unique_ptr<RasterizerTemplate> rasterizerState_;// ラスタライザーステートの管理クラス
		std::unique_ptr<BlendStateTemplate> blendStates_;// ブレンドステートの管理クラス
		std::unique_ptr<DepthStencilDescTemplate> depthStencilDescTemplate_;// 深度ステンシルステートの管理クラス
	};
}