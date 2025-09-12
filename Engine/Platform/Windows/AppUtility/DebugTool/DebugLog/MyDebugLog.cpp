#include "MyDebugLog.h"
#include "BuildInfo.h"

MyDebugLog::MyDebugLog() {
	Initialize();
}

MyDebugLog::~MyDebugLog() {
	Finalize();
}

void MyDebugLog::Initialize() {
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

void DebugLog(const std::string& message, const std::source_location& location) {
	MyDebugLog::GetInstance()->Log(message, location);
}

void DebugLogLua(const sol::object& obj) {
	if (obj.is<std::string>()) {
		DebugLog(obj.as<std::string>().c_str());
	} else if (obj.is<double>()) {
		DebugLog(std::to_string(obj.as<double>()).c_str());
	} else if (obj.is<int>()) {
		DebugLog(std::to_string(obj.as<int>()).c_str());
	} else if (obj.is<bool>()) {
		DebugLog(obj.as<bool>() ? "true" : "false");
	} else {
		DebugLog("<unsupported type>");
	}
}
