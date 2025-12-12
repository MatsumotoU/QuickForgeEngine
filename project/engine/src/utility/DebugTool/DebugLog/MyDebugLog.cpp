#include "MyDebugLog.h"
#include "BuildInfo.h"

MyDebugLog::MyDebugLog() {
	Initialize();
}

MyDebugLog::~MyDebugLog() {
	Finalize();
}

void MyDebugLog::Initialize() {
	log_.clear();
	engineLog_.clear();
	editorLog_.clear();
	warningLog_.clear();
	errorLog_.clear();

	std::filesystem::create_directory("logs");
	std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
	std::chrono::time_point<std::chrono::system_clock, std::chrono::seconds>
		nowSeconds = std::chrono::time_point_cast<std::chrono::seconds>(now);
	std::chrono::zoned_time localTime{ std::chrono::current_zone(),nowSeconds };
	std::string dateString = std::format("{:%Y-%m-%d_%H%M%S}", localTime);
	logFilePath_ = std::string("logs/") + dateString + ".logs";

	logStream_.open(logFilePath_);

	// ここでビルド情報を出力
	logStream_ << "Build Date: " << __DATE__ << " " << __TIME__ << std::endl;
#ifdef APP_VERSION
	logStream_ << "App Version: " << APP_VERSION << std::endl;
#endif
#ifdef BUILD_USER
	logStream_ << "Build User: " << BUILD_USER << std::endl;
#endif
#ifdef BUILD_BRANCH
	logStream_ << "Build Branch: " << BUILD_BRANCH << std::endl;
#endif
#ifdef BUILD_COMMIT
	logStream_ << "Build Commit: " << BUILD_COMMIT << std::endl;
#endif
#ifdef BUILD_PLATFORM
	logStream_ << "Build Platform: " << BUILD_PLATFORM << std::endl;
#endif
	logStream_ << "CreateLog" << std::endl;
}

void MyDebugLog::Finalize() {
	logStream_.close();
}

void MyDebugLog::Log(const std::string& message, const std::source_location& location) {
	std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
	std::chrono::time_point<std::chrono::system_clock, std::chrono::seconds>
		nowSeconds = std::chrono::time_point_cast<std::chrono::seconds>(now);
	std::chrono::zoned_time localTime{ std::chrono::current_zone(),nowSeconds };
	std::string timeStamp = std::format("{:%Y-%m-%d_%H-%M-%S}", localTime);

	std::string funcName = location.function_name();

	log_.push_back("[" + timeStamp + "] " + message);
	if (log_.size() > 300) {
		log_.erase(log_.begin());
	}

	logStream_ << "[" +  timeStamp + "] " + funcName+": " + message << std::endl;
	std::string logMessage = message + "\n";
	OutputDebugStringA(logMessage.c_str());
}

const std::vector<std::string>* MyDebugLog::GetLog() {
	return &log_;
}

void MyDebugLog::DebugLogClear() {
	engineLog_.clear();
	editorLog_.clear();
	warningLog_.clear();
	errorLog_.clear();
}

void DebugLog(const std::string& message, const LogLevel& logLevel, const std::source_location& location) {

	MyDebugLog::GetInstance()->Log(message, location);
	if (logLevel == LogLevel::EngineInfo) {
		MyDebugLog::GetInstance()->engineLog_.push_back(message);
		if (MyDebugLog::GetInstance()->engineLog_.size() > 300) {
			MyDebugLog::GetInstance()->engineLog_.erase(MyDebugLog::GetInstance()->engineLog_.begin());
		}
	} else if (logLevel == LogLevel::EditorInfo) {
		MyDebugLog::GetInstance()->editorLog_.push_back(message);
		if (MyDebugLog::GetInstance()->editorLog_.size() > 300) {
			MyDebugLog::GetInstance()->editorLog_.erase(MyDebugLog::GetInstance()->editorLog_.begin());
		}
	} else if (logLevel == LogLevel::Warning) {
		MyDebugLog::GetInstance()->warningLog_.push_back(message);
		if (MyDebugLog::GetInstance()->warningLog_.size() > 300) {
			MyDebugLog::GetInstance()->warningLog_.erase(MyDebugLog::GetInstance()->warningLog_.begin());
		}
	} else if (logLevel == LogLevel::Error) {
		MyDebugLog::GetInstance()->errorLog_.push_back(message);
		if (MyDebugLog::GetInstance()->errorLog_.size() > 300) {
			MyDebugLog::GetInstance()->errorLog_.erase(MyDebugLog::GetInstance()->errorLog_.begin());
		}
	}
}

void AppendLuaValueToString(const sol::object& v, std::string& msg) {
	if (v.is<std::string>()) {
		msg += v.as<std::string>();
	} else if (v.is<double>()) {
		msg += std::to_string(v.as<double>());
	} else if (v.is<int>()) {
		msg += std::to_string(v.as<int>());
	} else if (v.is<bool>()) {
		msg += v.as<bool>() ? "true" : "false";
	} else if (v.is<sol::nil_t>()) {
		msg += "nil";
	} else {
		msg += "<unsupported type>";
	}
}

void DebugLogLua(sol::variadic_args va, uint32_t id, const std::string& scriptName) {
	std::string msg;
	for (auto&& v : va) {
		AppendLuaValueToString(v, msg);
		msg += " ";
		MyDebugLog::GetInstance()->scriptLogs_[id][scriptName].push_back(msg);
		if (MyDebugLog::GetInstance()->scriptLogs_[id][scriptName].size() > 100) {
			MyDebugLog::GetInstance()->scriptLogs_[id][scriptName].erase(MyDebugLog::GetInstance()->scriptLogs_[id][scriptName].begin());
		}
	}
	DebugLog(msg, LogLevel::EditorInfo);
}

void DebugLogCsharp(const std::string& message) {
	DebugLog(message, LogLevel::EditorInfo);
}
