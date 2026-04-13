#pragma once
namespace QFE {
	/**
	 * @class SceneState
	 * @brief シーンの状態を表す列挙型
	 */
	enum class SceneState {
		FirstLoad,		/**< 最初のシーンロード */
		Running,		/**< シーンが実行中 */
		Transitioning,	/**< シーン遷移中 */
	};
}