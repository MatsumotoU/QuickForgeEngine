#include "MyDebugLog.h"
#include "EngineDefines.h"

#include <cassert>
#include <mutex>

using namespace QFE;

namespace {
	struct LoggerState {
		LoggerState() = default;
		LoggerState(const LoggerState&) = delete;
		LoggerState& operator=(const LoggerState&) = delete;
		LoggerState(LoggerState&&) = delete;
		LoggerState& operator=(LoggerState&&) = delete;
		std::mutex mutex;
		std::deque<LogEntry> entries;
		uint64_t nextSequence = 0;
	};
	LoggerState& GetLoggerState() { static LoggerState state; return state; }
}

void MyDebugLog::Initialize() {
	try
	{
		auto& state = GetLoggerState();
		std::scoped_lock lock(state.mutex);
		log_.clear();
		state.entries.clear();
		engineLog_.clear();
		editorLog_.clear();
		warningLog_.clear();
		errorLog_.clear();
		locationLogMap_.clear();

		std::filesystem::create_directory("logs");
		std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
		std::chrono::time_point<std::chrono::system_clock, std::chrono::seconds>
			nowSeconds = std::chrono::time_point_cast<std::chrono::seconds>(now);
		std::chrono::zoned_time localTime{ std::chrono::current_zone(),nowSeconds };
		std::string dateString = std::format("{:%Y-%m-%d_%H%M%S}", localTime);
		logFilePath_ = std::string("logs/") + dateString + ".logs";

		logStream_.open(logFilePath_);
		logStream_ << "CreateLog" << std::endl;
	}
	catch (const std::exception&)
	{
		QFE_REPORT_SYSTEM_ERROR("Failed to initialize MyDebugLog.",SystemError::Abort);
	}
}

void MyDebugLog::Finalize() {
	std::scoped_lock lock(GetLoggerState().mutex);
	logStream_.close();
}

void MyDebugLog::Log(const std::string& message, const std::source_location& location, LogLevel level) {
	try
	{
		auto& state = GetLoggerState();
		std::scoped_lock lock(state.mutex);
		// ログ出力の際に、時間と関数名を付与する
		std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
		std::chrono::time_point<std::chrono::system_clock, std::chrono::seconds>
			nowSeconds = std::chrono::time_point_cast<std::chrono::seconds>(now);
		std::chrono::zoned_time localTime{ std::chrono::current_zone(),nowSeconds };
		std::string timeStamp = std::format("{:%Y-%m-%d_%H-%M-%S}", localTime);
		state.entries.push_back({ ++state.nextSequence, level, timeStamp, message });
		if (state.entries.size() > 3000) state.entries.pop_front();
		auto* category = &engineLog_;
		if (level == LogLevel::EditorInfo) category = &editorLog_;
		else if (level == LogLevel::Warning) category = &warningLog_;
		else if (level == LogLevel::Error) category = &errorLog_;
		category->push_back(message);
		if (category->size() > 300) category->erase(category->begin());

		// ログの保存と、ログファイルへの出力
		log_.push_back("[" + timeStamp + "] " + message);
		if (log_.size() > 300) {
			log_.erase(log_.begin());
		}

		// 関数名を取得
		std::string funcName = location.function_name();

		// ログファイルへの出力
		logStream_ << "[" + timeStamp + "] " + funcName + ": " + message << std::endl;
		std::string logMessage = message + "\n";
		OutputDebugStringA(logMessage.c_str());

		// ログの場所ごとに分類されたログに保存
		std::string className = location.file_name();
		locationLogMap_[className][funcName].push_back(message);
		if (locationLogMap_[className][funcName].size() > 300) {
			locationLogMap_[className][funcName].pop_front();
		}
	}
	catch (const std::exception&)
	{
		assert(false && "faild to write logFile.");
	}
}

std::vector<LogEntry> MyDebugLog::GetEntries() {
	auto& state = GetLoggerState();
	std::scoped_lock lock(state.mutex);
	return { state.entries.begin(), state.entries.end() };
}

void MyDebugLog::ClearEntries() {
	auto& state = GetLoggerState();
	std::scoped_lock lock(state.mutex);
	state.entries.clear();
}

const std::vector<std::string>* MyDebugLog::GetLog() {
	return &log_;
}

void MyDebugLog::DebugLogClear() {
	std::scoped_lock lock(GetLoggerState().mutex);
	engineLog_.clear();
	editorLog_.clear();
	warningLog_.clear();
	errorLog_.clear();
}

void QFE::MyDebugLog::ClearLocationLogs()
{
	std::scoped_lock lock(GetLoggerState().mutex);
	// ログを消す前にどのくらい容量を使っているかを出力する
	size_t totalSize = 0;
	for (const auto& [className, funcMap] : locationLogMap_) {
		for (const auto& [funcName, logs] : funcMap) {
			totalSize += logs.size();
		}
	}
	// ログを消す前の容量を出力する
	std::string logMessage = "ClearLocationLogs: totalSize = " + std::to_string(totalSize) + "\n";
	OutputDebugStringA(logMessage.c_str());

	// ログを消す
	locationLogMap_.clear();
}

void QFE::DebugLog(const std::string& message, const LogLevel& logLevel, const std::source_location& location) {

	MyDebugLog::GetInstance()->Log(message, location, logLevel);
}

void QFE::DebugLogCsharp(const std::string& message) {
	QFE_LOG(message, LogLevel::EditorInfo);
}
