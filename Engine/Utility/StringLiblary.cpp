#include "StringLiblary.h"

#ifdef _DEBUG
#include "Base/EngineCore.h"
#include "Utility/MyDebugLog.h"
#endif // _DEBUG

StringLiblary::StringLiblary() {
	liblary_.clear();
	liblaryFriendryName_ = "";
}

StringLiblary::~StringLiblary() {
#ifdef _DEBUG
	DebugLog(std::format("=====LiblaryListLog from {}=====",liblaryFriendryName_));
	uint32_t index = 0;
	for (std::string& str : liblary_) {
			DebugLog(std::format("Data[{}]: {}",index, str));
			index++;
	}
	DebugLog("========================");
#endif // _DEBUG
}

void StringLiblary::Init(const std::string& libraryFriendName) {
	liblary_.clear();
	liblaryFriendryName_ = libraryFriendName;
#ifdef _DEBUG
	DebugLog(std::format("Create Liblary Name: {}", liblaryFriendryName_));
#endif // _DEBUG
}

void StringLiblary::AddStringToLiblary(const std::string& string) {
	if (FindString(string)) {
#ifdef _DEBUG
		DebugLog(std::format("[{}] already loaded.",string));
#endif // _DEBUG

	} else {
#ifdef _DEBUG
		DebugLog(std::format("Add string to liblary [{}].", string));
#endif // _DEBUG
		liblary_.push_back(string);
	}
}

bool StringLiblary::FindString(const std::string& string) {
	for (std::string& str : liblary_) {
		if (str == string) {
#ifdef _DEBUG
			DebugLog(std::format("Find [{}].", string));
#endif // _DEBUG
			return true;
		}
	}

#ifdef _DEBUG
	DebugLog(std::format("Not find [{}].", string));
#endif // _DEBUG
	return false;
}

int32_t StringLiblary::GetLiblaryIndex(const std::string& string) {
	uint32_t indexCount = 0;
	for (std::string& str : liblary_) {
		if (str == string) {
#ifdef _DEBUG
			DebugLog(std::format("Find {} Index: [{}].", string,indexCount));
#endif // _DEBUG
			return indexCount;
		}
		indexCount++;
	}
	return -1;
}

std::string StringLiblary::GetDatanameFromIndex(uint32_t index) {
	std::string result = "None";
	if (static_cast<uint32_t>(liblary_.size()) <= index) {
		return result;
	}

	result = liblary_[index];
	return result;
}

#ifdef _DEBUG
void StringLiblary::DrawLiblary() {
	uint32_t index = 0;
	for (std::string& str : liblary_) {
		ImGui::Text("Data[%d]: %s", index, str.c_str());
		index++;
	}
}
#endif // _]DEBUG