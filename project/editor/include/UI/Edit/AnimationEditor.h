#pragma once
#include "../IEditorUI.h"

#include "engine/include/assets/Animator/AnimClip.h"
#include <memory>

namespace QFE {
	/// @brief キーフレームの編集用構造体
	struct EditorKeyFrame {
		int frame; ///< キーフレーム
		Transform transform; ///< キーフレームのTransform
	};

	/// @brief アニメーションの設定を編集するパネル
	class AnimationEditor final : public IEditorUI {
	public:
		AnimationEditor();
		~AnimationEditor() = default;

		/** @brief 初期化 */
		void Initialize() override;
		/** @brief 更新 */
		void Update() override;
		/** @brief 描画 */
		void Draw() override;
		/** @brief 実行時処理 */
		void Run() override;
	private:
		int currentFrame_ = 0; ///< 現在のフレーム
		int frameStart_ = 0; ///< フレームの開始
		int frameEnd_ = 100; ///< フレームの終了
		float fps_ = 60.0f; ///< フレームレート

		bool isTransformOpen_ = false; ///< Transformグループの開閉状態

		std::vector<EditorKeyFrame> editingKeyFrames_; ///< 編集中のキーフレームのリスト
		int selectedKeyIndex_ = -1; ///< 選択中のキーフレームのインデックス

		std::unique_ptr<AnimClip> animClip_; ///< 編集結果を保存するAnimClip
		char animFileNameBuffer_[256]; ///< ファイル名入力用バッファ
	};
}