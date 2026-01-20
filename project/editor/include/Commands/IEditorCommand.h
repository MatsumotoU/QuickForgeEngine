#pragma once
#include <string>
#include <vector>

/** @class IEditorCommand
 *  @brief エディタのコマンドのインターフェース
 */
class IEditorCommand {
public:
	/** @brief コンストラクタ */
	IEditorCommand() = delete;
	explicit IEditorCommand(
		std::vector<std::string>& consoleLog) : cons_(consoleLog) {}

	virtual ~IEditorCommand() = default;
	/** @brief コマンドを実行する */
	virtual void Execute() = 0;
	/** @brief コマンドを元に戻す */
	virtual void Undo() = 0;
	/** @brief コマンドの略称一覧 */
	virtual const std::vector<std::string> GetAliases() const = 0;

protected:
	std::vector<std::string>& cons_;
};