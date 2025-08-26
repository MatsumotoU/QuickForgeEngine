#pragma once
#include <Windows.h>
#include <filesystem>
#include <fstream>
#include <chrono>
#include <source_location>
#include <vector>
#include <sol/sol.hpp>

#include "DesignPatterns/Singleton.h"

class MyDebugLog : public Singleton<MyDebugLog> {
    friend class Singleton<MyDebugLog>;
public:
    void Log(const std::string& message, const std::source_location& location = std::source_location::current());
	const std::vector<std::string>* GetLog();
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
void DebugLog(const std::string& message, const std::source_location& location = std::source_location::current());
// 文字列・数値どちらも受け取れるラッパー関数
void DebugLogLua(const sol::object& obj);