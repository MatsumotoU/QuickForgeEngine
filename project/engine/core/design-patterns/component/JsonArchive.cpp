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

void QFE::JsonArchive::Process(const std::string& name, MATH::Vector2& value) {
    if (isLoading_) {
        // デシリアライズ
        if (json_.contains(name)) {
            value.x = json_[name]["x"].get<float>();
            value.y = json_[name]["y"].get<float>();
        }
    } else {
        // シリアライズ
        json_[name]["x"] = value.x;
        json_[name]["y"] = value.y;
	}
}

void QFE::JsonArchive::Process(const std::string& name, MATH::Vector3& value) {
    if (isLoading_) {
        // デシリアライズ
        if (json_.contains(name)) {
            value.x = json_[name]["x"].get<float>();
            value.y = json_[name]["y"].get<float>();
            value.z = json_[name]["z"].get<float>();
        }
    } else {
        // シリアライズ
        json_[name]["x"] = value.x;
        json_[name]["y"] = value.y;
        json_[name]["z"] = value.z;
    }
}

void QFE::JsonArchive::Process(const std::string& name, MATH::Vector4& value) {
    if (isLoading_) {
        // デシリアライズ
        if (json_.contains(name)) {
            value.x = json_[name]["x"].get<float>();
            value.y = json_[name]["y"].get<float>();
            value.z = json_[name]["z"].get<float>();
            value.w = json_[name]["w"].get<float>();
        }
    } else {
        // シリアライズ
        json_[name]["x"] = value.x;
        json_[name]["y"] = value.y;
        json_[name]["z"] = value.z;
        json_[name]["w"] = value.w;
    }
}
