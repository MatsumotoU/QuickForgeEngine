#include "JsonArchive.h"

void QFE::JsonArchive::Process(const std::string& name, float& value) {
    if (isLoading_) {
		// デシリアライズ
        if (json_.contains(name)) value = json_[name].get<float>();
    } else {
		// シリアライズ
        json_[name] = value;
    }
}

void QFE::JsonArchive::Process(const std::string& name, int32_t& value) {
    if (isLoading_) {
		// デシリアライズ
        if (json_.contains(name)) value = json_[name].get<int32_t>();
    } else {
		// シリアライズ
        json_[name] = value;
    }
}

void QFE::JsonArchive::Process(const std::string& name, uint32_t& value) {
    if (isLoading_) {
        // デシリアライズ
        if (json_.contains(name)) value = json_[name].get<uint32_t>();
    } else {
        // シリアライズ
        json_[name] = value;
	}
}

void QFE::JsonArchive::Process(const std::string& name, std::string& value) {
    if (isLoading_) {
		// デシリアライズ
        if (json_.contains(name)) value = json_[name].get<std::string>();
    } else {
		// シリアライズ
        json_[name] = value;
    }
}
