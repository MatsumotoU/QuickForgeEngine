#pragma once
#include <Windows.h>
#include <filesystem>
#include <fstream>
#include <chrono>
#include <source_location>
#include <vector>
#include <sol/sol.hpp>
#include <string>
#include <unordered_set>
#include <unordered_map>

#include "DesignPatterns/Singleton.h"

enum class LogLevel {
	EngineInfo,
	EditorInfo,
	Warning,
	Error
};

class MyDebugLog : public Singleton<MyDebugLog> {
    friend class Singleton<MyDebugLog>;
public:
    void Log(const std::string& message, const std::source_location& location = std::source_location::current());
	const std::vector<std::string>* GetLog();
	void DebugLogClear();

	std::vector<std::string> engineLog_;
	std::vector<std::string> editorLog_;
	std::vector<std::string> warningLog_;
	std::vector<std::string> errorLog_;

	std::unordered_map<uint32_t, std::unordered_map<std::string,std::vector<std::string>>> scriptLogs_;
private:
    MyDebugLog();
    ~MyDebugLog() override;
	void Initialize();
	void Finalize();

	std::ofstream logStream_;
	std::string logFilePath_;
	std::vector<std::string> log_;
};

/// <summary>
/// このヘッダーを読み込んでいれば使えるやつ
/// </summary>
/// <param name="message"></param>
void DebugLog(const std::string& message,const LogLevel& logLevel = LogLevel::EngineInfo, const std::source_location& location = std::source_location::current());

// Luaの値を文字列に変換して追加する関数
void AppendLuaValueToString(const sol::object& v, std::string& msg);

// 文字列・数値どちらも受け取れるラッパー関数
void DebugLogLua(sol::variadic_args va,uint32_t id,const std::string& scriptName);