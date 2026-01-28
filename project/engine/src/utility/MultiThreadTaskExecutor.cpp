/**
 * @file MultiThreadTaskExecutor.cpp
 * @brief 複数の同時実行タスクを管理し、完了したタスクを自動クリーンアップするクラスの実装
 */

#include "engine/include/utility/MultiThreadTaskExecutor.h"
#include <vector>

namespace QFE {

	/** @brief 初期化 */
	void MultiThreadTaskExecutor::Initialize() {
		nextTaskId_ = 0;
		taskMap_.clear();
	}

	/** @brief フレーム終了時の処理 (完了タスクのクリーンアップ) */
	void MultiThreadTaskExecutor::FrameEnd() {
		// 完了したタスクのIDを収集
		std::vector<uint32_t> completedTasks;
		for (auto& [id, task] : taskMap_) {
			if (task.IsSuccess()) {
				completedTasks.push_back(id);
			}
		}
		// 完了したタスクを削除
		for (auto id : completedTasks) {
			taskMap_.erase(id);
		}
	}

	/** @brief 終了処理 */
	void MultiThreadTaskExecutor::Finalize() {
		for (auto& [id, task] : taskMap_) {
			if (task.IsStarted()) {
				while (task.IsRunning()) {
					std::this_thread::sleep_for(std::chrono::milliseconds(1));
				}
			}
			task.Init();
		}
		taskMap_.clear();
	}

	/**
	 * @brief 新規タスクの追加と実行開始
	 * @param task 実行する関数
	 * @return タスクハンドル
	 */
	uint32_t MultiThreadTaskExecutor::AddTask(const std::function<void()>& task) {
		uint32_t handle = nextTaskId_++;
		taskMap_[handle].Init();
		taskMap_[handle].Start(task);
		return handle;
	}

}
