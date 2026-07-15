#pragma once
#include <string>

namespace QFE::GRAPHIC {
	/// @brief 深度値比較方法
	enum class DepthStencilDescType : uint32_t {
		Default = 0,// 標準、比較あり、書き込みあり
		Translucent,// 半透明につかう.比較あり、書きこみなし
		None,// UIとかに使う.比較も書き込みもしない
		kCount // タイプの数,使用しない
	};

	/// @brief ブレンドモードの種類を定義する列挙型
	enum class BlendMode : uint32_t {
		// ブレンドなし
		kBlendModeNone = 0,
		// 通常
		kBlendModeNormal,
		// 加算
		kBlendModeAdd,
		// 減算
		kBlendModeSubtract,
		// 乗算
		kBlendModeMultiply,
		// スクリーン
		kBlendModeScreen,
		// 利用禁止
		kCountOfBlendMode,
	};

	/// @brief RasterizerStateの設定の種類を表す列挙型
	enum class RasterizerType : uint32_t {
		Default = 0,
		Wireframe,
		CullNone,
		kCount // タイプの数,使用しない
	};

	/// @brief BuiltInのシェーダーペアのタイプを定義
	enum class BuiltInShaderPair : uint32_t {
		ObjectMini = 0,
		Object2D,
		Object3D,
		Particle,
		Primitive,
		Skybox,
	};

	/// @brief シェーダーペアを生成するための情報をまとめた構造体
	struct ShaderPairElement {
		std::string vsDirName;
		std::string psDirName;
		std::string vsFileName;
		std::string psFileName;
	};
}