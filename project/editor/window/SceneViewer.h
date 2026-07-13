#pragma once
#include "IEditorWindow.h"
#include <imgui/imgui.h>

namespace QFE::EDITOR {
	/// @brief SceneViewerはシーンを表示します
	class SceneViewer final : public IEditorWindow {
	public:
		/// @brief SceneViewerのコンストラクタ
		SceneViewer(ImTextureID sceneTextureId);

		/// @brief ウィンドウの初期化処理
		void Initialize() override;
		/// @brief ウィンドウの描画処理
		void Draw(std::set<uint32_t>& selectedEntities, EditorCommandList& commandList) override;

		/// @brief ウィンドウの名前を取得する
		std::string GetWindowName() override;
		/// @brief ウィンドウがアクティブかどうかを取得する
		bool GetIsActive() override;
		/// @brief ウィンドウのアクティブ状態を設定する
		bool SetIsActive(bool isActive) override;
		/// @brief ウィンドウがフォーカスされているかどうかを取得する
		bool GetIsFocus() override;

	private:
		ImTextureID sceneTextureId_;
		bool isActive_;
		bool isFocus_;
	};
}
