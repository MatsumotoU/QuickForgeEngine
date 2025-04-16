#pragma once
#include <Windows.h>
#include <filesystem>
#include <fstream>
#include <chrono>

class MyDebugLog {
public:
	// シングルトン
	static MyDebugLog* GetInstatnce();

public:

	void Initialize();
	void Finalize();
	void Log(const std::string& message);

private:
	std::ofstream logStream_;
	std::string logFilePath_;

private: // シングルトン用
	MyDebugLog() = default;
	~MyDebugLog() = default;
	MyDebugLog(const MyDebugLog&) = delete;
	MyDebugLog& operator=(const MyDebugLog&) = delete;

};