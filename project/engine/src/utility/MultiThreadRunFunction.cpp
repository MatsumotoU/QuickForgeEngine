/**
 * @file MultiThreadRunFunction.cpp
 * @brief 単一の関数を別スレッドで実行するための軽量ラッパークラスの実装
 */

#include "engine/include/utility/MultiThreadRunFunction.h"

namespace QFE {

	MultiThreadRunFunction::MultiThreadRunFunction() : isRunning(false), isStarted(false) {}

	/** @brief デストラクタ (実行中なら終了を待機) */
	MultiThreadRunFunction::~MultiThreadRunFunction() {
		if (t.joinable()) {
			t.join();
		}
	}

	/** @brief 初期化・リセット */
	void MultiThreadRunFunction::Init() {
		isRunning = false;
		isStarted = false;
		if (t.joinable()) {
			t.join();
		}
	}

	/**
	 * @brief 関数の実行開始
	 * @param func 実行する関数
	 */
	void MultiThreadRunFunction::Start(std::function<void()> func) {
		if (isRunning || isStarted) {
			return;
		}

		isRunning = true;
		isStarted = true;
		t = std::thread([this, func]() {
			func();
			isRunning = false;
			});
	}

	/** @brief 実行を開始したかを取得 */
	const bool MultiThreadRunFunction::IsStarted() const {
		return isStarted;
	}

	/** @brief 現在実行中かを取得 */
	const bool MultiThreadRunFunction::IsRunning() const {
		return isRunning;
	}

	/** @brief 実行が正常に完了したかを取得 */
	const bool MultiThreadRunFunction::IsSuccess() const {
		return (!isRunning && isStarted);
	}

}
