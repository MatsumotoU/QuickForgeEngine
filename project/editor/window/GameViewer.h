#pragma once
#include "IEditorWindow.h"
#include <imgui/imgui.h>
#include "math/MathInclude.h"

namespace QFE::SCENE { class SceneManager; }

namespace QFE::EDITOR {
	/// @brief GameViewerはシーンを表示します
	class GameViewer final : public IEditorWindow {
	public:
		/// @brief GameViewerのコンストラクタ
		GameViewer(ImTextureID sceneTextureId, SCENE::SceneManager* sceneManager, unsigned width, unsigned height);
		/// @brief ドロップ位置の算出に使用するカメラ行列を設定する。
		void SetAssetDropViewProjection(const MATH::Matrix4x4& viewProjection);

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
		SCENE::SceneManager* sceneManager_;
		unsigned width_;
		unsigned height_;
		MATH::Matrix4x4 dropViewProjection_ = MATH::Matrix4x4::MakeIdentity4x4();
		bool isActive_;
		bool isFocus_ = false;
	};
}
