#pragma once
#include <cstdint>
#include <chrono>
#include <array>

namespace QFE {
	/// @brief FPSを計測するクラス
	class FPSCounter {
	public:
		/// @brief すべての値をリセットします。
		void Reset();

		/// @brief フレームの開始を通知します。
		void FrameStart();
		/// @brief フレームの終了を通知します。
		void FrameEnd();
		/// @brief 現在のFPSを取得します。
		float GetCurrentFPS() const;
		/// @brief 過去100フレームの平均FPSを取得します。
		float GetAverageFPS() const;

	private:
		std::chrono::microseconds frameStartTime_; // フレームの開始時間

		float elapsedTime_; // 経過時間
		float currentFPS_;       // 現在のFPS

		uint32_t currentFrameIndex_; // 現在のフレームインデックス
		std::array<float, 100> averageFPS_; // 過去100フレームのFPSを保持する配列
	};
}