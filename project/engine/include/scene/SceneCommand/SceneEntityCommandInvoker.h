#pragma once
#include "ISceneEntityCommand.h"
#include <queue>
#include <memory>
namespace QFE {
	/// @brief シーンエンティティに対するコマンドを管理・実行するクラス
	class SceneEntityCommandInvoker final {
	private:
		float commandTimeout_;

		std::queue<std::unique_ptr<ISceneEntityCommand>> systemCommandQueue_;// このエンジンで自動的に発行されるコマンドキュー
		std::queue<std::unique_ptr<ISceneEntityCommand>> userCommandQueue_;// ユーザーが発行するコマンドキュー

	public:
		// コンストラクタ・デストラクタ
		SceneEntityCommandInvoker() = delete;
		explicit SceneEntityCommandInvoker(float commandTimeout);
		~SceneEntityCommandInvoker() = default;
		// コピーとムーブを禁止
		SceneEntityCommandInvoker(const SceneEntityCommandInvoker&) = delete;
		SceneEntityCommandInvoker& operator=(const SceneEntityCommandInvoker&) = delete;
		SceneEntityCommandInvoker(SceneEntityCommandInvoker&&) = delete;
		SceneEntityCommandInvoker& operator=(SceneEntityCommandInvoker&&) = delete;

		/// @brief システム側で発行したコマンドを追加する
		void AddSystemCommand(std::unique_ptr<ISceneEntityCommand> command);
		/// @brief ユーザー側で発行したコマンドを追加する
		void AddUserCommand(std::unique_ptr<ISceneEntityCommand> command);

		/// @brief コマンドを実行する
		void ExecuteCommands();
		/// @brief コマンドキューをクリアする
		void ClearCommands();
	};
}  // namespace QFE