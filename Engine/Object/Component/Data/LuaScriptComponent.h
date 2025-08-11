#pragma once
#include <sol/sol.hpp>
#include <string>
#include <vector>

struct LuaScriptComponent {
	std::string scriptName;// 保存用
	std::vector<std::string> funcNames; // スクリプト内の関数名を保存
	std::vector<std::string> valueNames; // スクリプト内の変数名を保存
};