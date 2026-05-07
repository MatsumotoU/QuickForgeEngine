#pragma once
#include <string>

namespace QFE::Script {
	/// @brief スクリプトをコンパイルする
	extern void CompileScripts();
	/// @brief 決められた名前でスクリプトのテンプレートを作成する
	extern void CreateScriptTemplate(const std::string& scriptName,const std::string& outputPath);
}