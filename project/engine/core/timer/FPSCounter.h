#pragma once
#include <array>

namespace QFE {
	/// @brief FPSを計測するクラス
	class FPSCounter {
	public:
		/// @brief FPSを更新します。1秒ごとにFPSを計算します。
		void Update(float deltaTime);
		/// @brief 現在のFPSを取得します。
		int GetFPS() const;
	private:
		float elapsedTime_ = 0.0f; // 経過時間
		int currentFPS_ = 0;       // 現在のFPS
	};
}