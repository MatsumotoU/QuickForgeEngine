#include "StringLibrary.h"

#include "../EngineDefines.h"

namespace QFE {

	StringLibrary::StringLibrary() {
		library_.clear();
		libraryFriendryName_ = "";
	}

	StringLibrary::~StringLibrary() {
		QFE_LOG(std::format("=====LibraryListLog from {}=====", libraryFriendryName_));
		uint32_t index = 0;
		for (std::string& str : library_) {
			QFE_LOG(std::format("Data[{}]: {}", index, str));
			index++;
		}
		QFE_LOG("========================");
	}

	void StringLibrary::Init(const std::string& libraryFriendName) {
		library_.clear();
		libraryFriendryName_ = libraryFriendName;
		QFE_LOG(std::format("Create Library Name: {}", libraryFriendryName_));
	}

	void StringLibrary::AddStringToLibrary(const std::string& string) {
		if (FindString(string)) {
			QFE_LOG(std::format("[{}] already loaded.", string));

		} else {
			QFE_LOG(std::format("Add string to liblary [{}].", string));
			library_.push_back(string);
		}
	}

	bool StringLibrary::FindString(const std::string& string) {
		for (std::string& str : library_) {
			if (str == string) {
				QFE_LOG(std::format("Find [{}].", string));
				return true;
			}
		}

		QFE_LOG(std::format("Not find [{}].", string));
		return false;
	}

	int32_t StringLibrary::GetLibraryIndex(const std::string& string) {
		uint32_t indexCount = 0;
		for (std::string& str : library_) {
			if (str == string) {
				QFE_LOG(std::format("Find {} Index: [{}].", string, indexCount));
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
