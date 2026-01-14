/**
 * @file DebugConsole.h
 * @brief エディタ内からコマンドを実行できるコンパネの実装
 */

#pragma once
#include "../IEditorUI.h"
#include <vector>

/**
 * @class DebugConsole
 * @brief テキストベースのコマンドを入力・実行し、結果を表示するUI
 */
class DebugConsole final : public IEditorUI {
public:
	DebugConsole() = default;
	~DebugConsole() = default;
	/** @brief 初期化 */
	void Initialize() override;
	/** @brief 更新 */
	void Update() override;
	/** @brief 描画 */
	void Draw() override;
	/** @brief 実行時処理 */
	void Run() override;
private:
	char inputBuf_[256] = {}; ///< コマンド入力バッファ
	std::vector<std::string> items_; ///< 過去のコマンドや出力結果の履歴

	/** @brief コマンドの実行 */
	void ExecCommand(const char* command);
};
