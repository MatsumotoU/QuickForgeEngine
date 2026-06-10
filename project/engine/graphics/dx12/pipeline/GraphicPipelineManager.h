#pragma once
#include <functional>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxgidebug.h>
#include <dxcapi.h>
#include <string>

#include <map>
#include <memory>

#include "../Descriptors/DsvDescriptorHeap.h"

namespace QFE::GRAPHIC::INTERNAL {
	/// @brief グラフィックスパイプラインを作るためのクラス群の前方宣言
	class ShaderReflection;
	class StaticSamplers;
	class ShaderPair;
	class RasterizerTemplate;
	class BlendStates;
	class DepthStencilDescTemplate;
	class PipelineStateObject;

	/// @brief シェーダーペアを生成するための情報をまとめた構造体
	struct ShaderPairElement {
		std::string vsDirName;
		std::string psDirName;
		std::string vsFileName;
		std::string psFileName;
	};

	/// @brief シェーダーペアとパイプラインステートオブジェクトのハンドルを定義
	enum class ShaderPairHandle : uint32_t {};
	enum class PipelineStateObjectHandle : uint32_t {};

	/// @brief グラフィックスパイプラインおよびルートシグネチャの管理クラス
	class GraphicPipelineManager final {
	public:
		explicit GraphicPipelineManager();
		~GraphicPipelineManager();

	public:
		/// @brief 初期化処理
		void Initialize(std::function<IDxcBlob*(const std::wstring&, const wchar_t*)> compileFunc, ID3D12Device* device);
		/// @brief 終了処理
		void Finalize();

		/// @brief シェーダーペアを生成します
		ShaderPairHandle GenerateShaderPair(
			const ShaderPairElement& element, std::function<IDxcBlob* (const std::wstring&, const wchar_t*)> compileFunc);
		/// @brief パイプラインステートオブジェクトを生成します
		PipelineStateObjectHandle GeneratePipelineStateObject(
			const ShaderPairHandle& shaderHandle, ID3D12Device* device
			, D3D12_PRIMITIVE_TOPOLOGY_TYPE topologyType, D3D12_RASTERIZER_DESC rasterizerDesc,
			D3D12_BLEND_DESC blendDesc, D3D12_DEPTH_STENCIL_DESC depthStencilDesc);

	private: // メンバ変数
		uint32_t shaderPairKeyCounter_ = 0;// シェーダーペアのキーを管理するカウンター
		std::map<uint32_t, std::unique_ptr<ShaderPair>> shaderPairs_;// シェーダーペアのマップ
		uint32_t pipelineStateObjectKeyCounter_ = 0;// パイプラインステートオブジェクトのキーを管理するカウンター
		std::map<uint32_t, std::unique_ptr<PipelineStateObject>> pipelineStateObjects_;// パイプラインステートオブジェクトのマップ

		std::unique_ptr<ShaderReflection> shaderReflection_;// シェーダーのリフレクションを行うためのクラス
		std::unique_ptr<StaticSamplers> staticSamplers_;// 静的サンプラーの管理クラス
		std::unique_ptr<RasterizerTemplate> rasterizerState_;// ラスタライザーステートの管理クラス
		std::unique_ptr<BlendStates> blendStates_;// ブレンドステートの管理クラス
		std::unique_ptr<DepthStencilDescTemplate> depthStencilDescTemplate_;// 深度ステンシルステートの管理クラス
	};
}