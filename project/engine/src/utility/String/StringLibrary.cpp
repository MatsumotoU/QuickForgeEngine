#include "engine/include/utility/String/StringLibrary.h"

#ifdef QFE_OPTIMIZE_OFF
#include "engine/include/utility/DebugTool/DebugLog/MyDebugLog.h"
#endif // QFE_OPTIMIZE_OFF

namespace QFE {

	StringLibrary::StringLibrary() {
		library_.clear();
		libraryFriendryName_ = "";
	}

	StringLibrary::~StringLibrary() {
#ifdef QFE_OPTIMIZE_OFF
		QFE_LOG(std::format("=====LibraryListLog from {}=====", libraryFriendryName_));
		uint32_t index = 0;
		for (std::string& str : library_) {
			QFE_LOG(std::format("Data[{}]: {}", index, str));
			index++;
		}
		QFE_LOG("========================");
#endif // QFE_OPTIMIZE_OFF
	}

	void StringLibrary::Init(const std::string& libraryFriendName) {
		library_.clear();
		libraryFriendryName_ = libraryFriendName;
#ifdef QFE_OPTIMIZE_OFF
		QFE_LOG(std::format("Create Library Name: {}", libraryFriendryName_));
#endif // QFE_OPTIMIZE_OFF
	}

	void StringLibrary::AddStringToLibrary(const std::string& string) {
		if (FindString(string)) {
#ifdef QFE_OPTIMIZE_OFF
			QFE_LOG(std::format("[{}] already loaded.", string));
#endif // QFE_OPTIMIZE_OFF

		} else {
#ifdef QFE_OPTIMIZE_OFF
			QFE_LOG(std::format("Add string to liblary [{}].", string));
#endif // QFE_OPTIMIZE_OFF
			library_.push_back(string);
		}
	}

	bool StringLibrary::FindString(const std::string& string) {
		for (std::string& str : library_) {
			if (str == string) {
#ifdef QFE_OPTIMIZE_OFF
				QFE_LOG(std::format("Find [{}].", string));
#endif // QFE_OPTIMIZE_OFF
				return true;
			}
		}

#ifdef QFE_OPTIMIZE_OFF
		QFE_LOG(std::format("Not find [{}].", string));
#endif // QFE_OPTIMIZE_OFF
		return false;
	}

	int32_t StringLibrary::GetLibraryIndex(const std::string& string) {
		uint32_t indexCount = 0;
		for (std::string& str : library_) {
			if (str == string) {
#ifdef QFE_OPTIMIZE_OFF
				QFE_LOG(std::format("Find {} Index: [{}].", string, indexCount));
#endif // QFE_OPTIMIZE_OFF
				return indexCount;
			}
			indexCount++;
		}
		return -1;
	}

	std::string StringLibrary::GetNameFromIndex(uint32_t index) {
		std::string result = "None";
		if (static_cast<uint32_t>(library_.size()) <= index) {
			return result;
		}

		result = library_[index];
		return result;
	}

}
