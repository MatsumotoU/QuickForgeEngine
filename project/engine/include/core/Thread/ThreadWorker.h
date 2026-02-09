#pragma once
#include <thread>
#include <functional>
#include <atomic>

namespace QFE {
	/// @brief 1つのスレッドを占有してタスクを実行するクラス
	class ThreadWorker final {
	public:
		/// @brief 生成時にスレッドを起動する
		ThreadWorker();
		/// @brief 終了時にスレッドを停止する
		~ThreadWorker();
		// コピーとムーブを禁止
		ThreadWorker(const ThreadWorker&) = delete;
		ThreadWorker& operator=(const ThreadWorker&) = delete;
		ThreadWorker(ThreadWorker&&) = delete;
		ThreadWorker& operator=(ThreadWorker&&) = delete;

		/// @brief タスクを追加する
		void AddTask(const std::function<void()>& task);

	private:
		std::thread workerThread_;
		std::atomic<bool> isRunning_;
		std::atomic<bool> hasTask_;
		std::function<void()> currentTask_;
	};
}