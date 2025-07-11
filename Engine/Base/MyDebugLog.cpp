#include "MyDebugLog.h"

MyDebugLog* MyDebugLog::GetInstatnce() {
	static MyDebugLog instance;
	return &instance;
}

void MyDebugLog::Initialize() {

	std::filesystem::create_directory("logs");
	std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
	std::chrono::time_point<std::chrono::system_clock,std::chrono::seconds>
	nowSeconds = std::chrono::time_point_cast<std::chrono::seconds>(now);
	std::chrono::zoned_time localTime{ std::chrono::current_zone(),nowSeconds };
	std::string dateString = std::format("{:%Y-%m-%d_%H%M%S}", localTime);
	logFilePath_ = std::string("logs/") + dateString + ".logs";

	logStream_.open(logFilePath_);
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

std::vector<std::string>* MyDebugLog::GetLog() {
	return &log_;
}

void DebugLog(const std::string& message, const std::source_location& location) {
	MyDebugLog* myDebugLog = MyDebugLog::GetInstatnce();
	myDebugLog->Log(message,location);
}
