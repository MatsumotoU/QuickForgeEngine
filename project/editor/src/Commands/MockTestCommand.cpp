#include "editor/include/Commands/MockTestCommand.h"
#include "engine/include/core/Math/MyMath.h"

QFE::MockTestCommand::MockTestCommand(std::vector<std::string>& consoleLog, const char* command)
	: IEditorCommand(consoleLog, command),
	aliases_({ "mockTest","mock" }) {
}

void QFE::MockTestCommand::Execute()
{
	// テストをするときはここに処理を追加していく
}

void QFE::MockTestCommand::Undo()
{
	// 戻る意味のある処理は特にないため空実装
}

const std::vector<std::string> QFE::MockTestCommand::GetAliases() const
{
	return aliases_;
}
