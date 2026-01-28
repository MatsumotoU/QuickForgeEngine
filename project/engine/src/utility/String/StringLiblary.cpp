#include "engine/include/utility/String/StringLiblary.h"

#ifdef QFE_OPTIMIZE_OFF
#include "engine/include/utility/DebugTool/DebugLog/MyDebugLog.h"
#endif // QFE_OPTIMIZE_OFF

namespace QFE {

	StringLibrary::StringLibrary() {
		liblary_.clear();
		liblaryFriendryName_ = "";
	}

	StringLibrary::~StringLibrary() {
#ifdef QFE_OPTIMIZE_OFF
		DebugLog(std::format("=====LiblaryListLog from {}=====", liblaryFriendryName_));
		uint32_t index = 0;
		for (std::string& str : liblary_) {
			DebugLog(std::format("Data[{}]: {}", index, str));
			index++;
		}
		DebugLog("========================");
#endif // QFE_OPTIMIZE_OFF
	}

	void StringLibrary::Init(const std::string& libraryFriendName) {
		liblary_.clear();
		liblaryFriendryName_ = libraryFriendName;
#ifdef QFE_OPTIMIZE_OFF
		DebugLog(std::format("Create Liblary Name: {}", liblaryFriendryName_));
#endif // QFE_OPTIMIZE_OFF
	}

	void StringLibrary::AddStringToLiblary(const std::string& string) {
		if (FindString(string)) {
#ifdef QFE_OPTIMIZE_OFF
			DebugLog(std::format("[{}] already loaded.", string));
#endif // QFE_OPTIMIZE_OFF

		} else {
#ifdef QFE_OPTIMIZE_OFF
			DebugLog(std::format("Add string to liblary [{}].", string));
#endif // QFE_OPTIMIZE_OFF
			liblary_.push_back(string);
		}
	}

	bool StringLibrary::FindString(const std::string& string) {
		for (std::string& str : liblary_) {
			if (str == string) {
#ifdef QFE_OPTIMIZE_OFF
				DebugLog(std::format("Find [{}].", string));
#endif // QFE_OPTIMIZE_OFF
				return true;
			}
		}

#ifdef QFE_OPTIMIZE_OFF
		DebugLog(std::format("Not find [{}].", string));
#endif // QFE_OPTIMIZE_OFF
		return false;
	}

	int32_t StringLibrary::GetLiblaryIndex(const std::string& string) {
		uint32_t indexCount = 0;
		for (std::string& str : liblary_) {
			if (str == string) {
#ifdef QFE_OPTIMIZE_OFF
				DebugLog(std::format("Find {} Index: [{}].", string, indexCount));
#endif // QFE_OPTIMIZE_OFF
				return indexCount;
			}
			indexCount++;
		}
		return -1;
	}

	std::string StringLibrary::GetDatanameFromIndex(uint32_t index) {
		std::string result = "None";
		if (static_cast<uint32_t>(liblary_.size()) <= index) {
			return result;
		}

		result = liblary_[index];
		return result;
	}

}
