#pragma once
#include "math/MathInclude.h"
#include <imgui.h>
#include <string>

namespace QFE::SCENE { class SceneManager; }
namespace QFE::EDITOR {
	class EditorCommandList;
	/// @brief Asset Browserから渡されるアセットパスのドラッグ形式。
	inline constexpr const char* AssetPayloadType = "QFE_PROJECT_ASSET";
	/// @brief ドラッグ可能なプロジェクト相対パスか判定する。
	bool IsDraggableAsset(const std::string& relativePath);
	/// @brief 直前に描画したゲーム画像へのドロップを受け、Entity追加コマンドを登録する。
	void AcceptAssetDrop(SCENE::SceneManager* sceneManager, EditorCommandList& commandList,
		const ImVec2& imagePosition, const ImVec2& imageSize, unsigned width, unsigned height,
		const MATH::Matrix4x4& view, const MATH::Matrix4x4& projection);
}
