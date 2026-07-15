#include "FPSCounter.h"
#include <time.h>

void QFE::FPSCounter::Reset() {
	elapsedTime_ = 0.0f;
	currentFPS_ = 0.0f;
	currentFrameIndex_ = 0;
	averageFPS_.fill(0.0f);
}

void QFE::FPSCounter::FrameStart() {
	// フレームの開始時間を取得
	frameStartTime_ = std::chrono::duration_cast<std::chrono::microseconds>(
		std::chrono::high_resolution_clock::now().time_since_epoch());
}

void QFE::FPSCounter::FrameEnd() {
	// フレームの終了時間を取得
	auto frameEndTime = std::chrono::duration_cast<std::chrono::microseconds>(
		std::chrono::high_resolution_clock::now().time_since_epoch());
	// 経過時間を計算
	elapsedTime_ = static_cast<float>((frameEndTime - frameStartTime_).count()) / 1000000.0f;
	if(elapsedTime_ <= 0.0f) {
		elapsedTime_ = 0.0001f; // 過去のフレームが非常に短い場合の保険
	}
	// FPSを計算
	currentFPS_ = 1.0f / elapsedTime_;

	// 過去100フレームのFPSを保持する配列に追加
	averageFPS_[currentFrameIndex_ % averageFPS_.size()] = currentFPS_;
	currentFrameIndex_++;
}

float QFE::FPSCounter::GetCurrentFPS() const {
	return currentFPS_;
}

float QFE::FPSCounter::GetAverageFPS() const {
	// 過去100フレームの平均FPSを計算
	float sum = 0.0f;
	for (const auto& fps : averageFPS_) {
		sum += fps;
	}
	// 平均FPSを返す
	return sum / static_cast<float>(averageFPS_.size());
}

float QFE::FPSCounter::GetDeltaTime() const {
	return elapsedTime_;
}
