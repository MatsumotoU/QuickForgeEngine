#pragma once
#include <deque>
#include <memory>
#include "engine/include/core/Thread/ThreadWorker.h"

namespace QFE {
	/// @brief 複数のThreadWorkerを管理するスレッドプールクラス
	class ThreadPool final {
	public:
		/// @brief Maxスレッドから-1した数のスレッドで初期化する
		ThreadPool();
		/// @brief デストラクタ
		~ThreadPool() = default;
		// コピーとムーブを禁止
		ThreadPool(const ThreadPool&) = delete;
		ThreadPool& operator=(const ThreadPool&) = delete;
		ThreadPool(ThreadPool&&) = delete;
		ThreadPool& operator=(ThreadPool&&) = delete;

		/// @brief タスクを空いているスレッドに追加する
		/// @param task 実行するタスク
		/// @return タスクの追加に成功したらtrueを返す
		bool AddTask(const std::function<void()>& task);

		/// @brief 空いているスレッドがあるかどうかを返す
		bool HasIdleThread() const;
		/// @brief スレッドの数を返す
		size_t GetThreadCount() const;

	private:
		/// @brief ThreadWorkerのリスト
		std::deque<std::unique_ptr<ThreadWorker>> workers_;
	};
}