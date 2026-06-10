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

	/// @brief シェーダーペアのキーとなる文字列定数
	inline constexpr const std::string kObject3d = "QFE_Object3d";

	/// @brief シェーダーペアを生成するための情報をまとめた構造体
	struct ShaderPairElement {
		std::string vsDirName;
		std::string psDirName;
		std::string vsFileName;
		std::string psFileName;
		std::string pairKey;
	};

	/// @brief グラフィックスパイプラインおよびルートシグネチャの管理クラス
	class GraphicPipelineManager final {
	public:
		explicit GraphicPipelineManager();
		~GraphicPipelineManager();

	public:
		/// @brief 初期化処理
		void Initialize(std::function<IDxcBlob*(const std::wstring&, const wchar_t*)> compileFunc);
		/// @brief 終了処理
		void Finalize();

		/// @brief シェーダーペアを生成します
		void GenerateShaderPair(
			const ShaderPairElement& element, std::function<IDxcBlob* (const std::wstring&, const wchar_t*)> compileFunc);
	private: // メンバ変数
		std::map<std::string, std::unique_ptr<ShaderPair>> shaderPairs_;// シェーダーペアのマップ

		std::unique_ptr<ShaderReflection> shaderReflection_;// シェーダーのリフレクションを行うためのクラス
		std::unique_ptr<StaticSamplers> staticSamplers_;// 静的サンプラーの管理クラス
		std::unique_ptr<RasterizerTemplate> rasterizerState_;// ラスタライザーステートの管理クラス
		std::unique_ptr<BlendStates> blendStates_;// ブレンドステートの管理クラス
	};
}