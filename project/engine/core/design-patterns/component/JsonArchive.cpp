#include "JsonArchive.h"

void QFE::JsonArchive::Process(const std::string& name, bool& value) {
    if (isLoading_) {
        // デシリアライズ
        if (json_.contains(name)) value = json_[name].get<bool>();
    } else {
        // シリアライズ
        json_[name] = value;
	}
}

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

void QFE::JsonArchive::Process(const std::string& name, MATH::Transform& value) {
    if (isLoading_) {
        // デシリアライズ
        if (json_.contains(name)) {
			json_[name]["scale"]["x"].get_to(value.scale.x);
            json_[name]["scale"]["y"].get_to(value.scale.y);
            json_[name]["scale"]["z"].get_to(value.scale.z);
            json_[name]["rotate"]["x"].get_to(value.rotate.x);
            json_[name]["rotate"]["y"].get_to(value.rotate.y);
            json_[name]["rotate"]["z"].get_to(value.rotate.z);
            json_[name]["translate"]["x"].get_to(value.translate.x);
			json_[name]["translate"]["y"].get_to(value.translate.y);
			json_[name]["translate"]["z"].get_to(value.translate.z);
        }
    } else {
        // シリアライズ
		json_[name]["scale"]["x"] = value.scale.x;
        json_[name]["scale"]["y"] = value.scale.y;
        json_[name]["scale"]["z"] = value.scale.z;
        json_[name]["rotate"]["x"] = value.rotate.x;
        json_[name]["rotate"]["y"] = value.rotate.y;
        json_[name]["rotate"]["z"] = value.rotate.z;
        json_[name]["translate"]["x"] = value.translate.x;
        json_[name]["translate"]["y"] = value.translate.y;
		json_[name]["translate"]["z"] = value.translate.z;
	}
}
