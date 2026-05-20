#pragma once
#include "../IEditorUI.h"
#include "engine/include/assets/Animator/AnimClip.h"

#include "engine/include/utility/DebugTool/ImGui/ImSequencerWrapper.h"

namespace QFE {
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
	};
}