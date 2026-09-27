#include "JsonArchive.h"
#include "components/SplineControlPoint.h"

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

void QFE::JsonArchive::Process(const std::string& name, MATH::EulerTransform& value) {
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

void QFE::JsonArchive::Process(const std::string& name, std::vector<MATH::EulerTransform>& value) {
    if (isLoading_) {
        if (!json_.contains(name) || !json_[name].is_array()) {
            return;
        }

        std::vector<MATH::EulerTransform> loadedPoints;
        loadedPoints.reserve(json_[name].size());
        for (const nlohmann::json& pointJson : json_[name]) {
            MATH::EulerTransform point;
            if (pointJson.is_object()) {
                const auto readVector = [&pointJson](const char* key, MATH::Vector3& vector) {
                    const auto field = pointJson.find(key);
                    if (field == pointJson.end() || !field->is_object()) {
                        return;
                    }
                    vector.x = field->value("x", vector.x);
                    vector.y = field->value("y", vector.y);
                    vector.z = field->value("z", vector.z);
                };
                readVector("scale", point.scale);
                readVector("rotate", point.rotate);
                readVector("translate", point.translate);
            }
            loadedPoints.push_back(point);
        }
        value = std::move(loadedPoints);
    } else {
        nlohmann::json points = nlohmann::json::array();
        for (const MATH::EulerTransform& point : value) {
            points.push_back({
                { "scale", { { "x", point.scale.x }, { "y", point.scale.y }, { "z", point.scale.z } } },
                { "rotate", { { "x", point.rotate.x }, { "y", point.rotate.y }, { "z", point.rotate.z } } },
                { "translate", { { "x", point.translate.x }, { "y", point.translate.y }, { "z", point.translate.z } } }
            });
        }
        json_[name] = std::move(points);
    }
}

void QFE::JsonArchive::Process(const std::string& name, std::vector<SCENE::SplineControlPoint>& value) {
	if (isLoading_) {
		if (!json_.contains(name) || !json_[name].is_array()) {
			return;
		}

		// 旧形式のスプラインには全体のspeedしかないため、その値を各区間の初期値として引き継ぐ。
		const float legacySecondsToNextPoint = json_.value("speed", 1.0f);
		std::vector<SCENE::SplineControlPoint> loadedPoints;
		loadedPoints.reserve(json_[name].size());
		for (const nlohmann::json& pointJson : json_[name]) {
			SCENE::SplineControlPoint point{};
			point.secondsToNextPoint = pointJson.is_object()
				? pointJson.value("secondsToNextPoint", legacySecondsToNextPoint)
				: legacySecondsToNextPoint;

			if (pointJson.is_object()) {
				const nlohmann::json* transformJson = &pointJson;
				const auto transform = pointJson.find("transform");
				if (transform != pointJson.end() && transform->is_object()) {
					transformJson = &*transform;
				}

				const auto readVector = [transformJson](const char* key, MATH::Vector3& vector) {
					const auto field = transformJson->find(key);
					if (field == transformJson->end() || !field->is_object()) {
						return;
					}
					vector.x = field->value("x", vector.x);
					vector.y = field->value("y", vector.y);
					vector.z = field->value("z", vector.z);
				};
				readVector("scale", point.transform.scale);
				readVector("rotate", point.transform.rotate);
				readVector("translate", point.transform.translate);
			}
			loadedPoints.push_back(point);
		}
		value = std::move(loadedPoints);
	} else {
		nlohmann::json points = nlohmann::json::array();
		for (const SCENE::SplineControlPoint& point : value) {
			nlohmann::json pointJson;
			pointJson["transform"]["scale"] = {
				{ "x", point.transform.scale.x }, { "y", point.transform.scale.y }, { "z", point.transform.scale.z }
			};
			pointJson["transform"]["rotate"] = {
				{ "x", point.transform.rotate.x }, { "y", point.transform.rotate.y }, { "z", point.transform.rotate.z }
			};
			pointJson["transform"]["translate"] = {
				{ "x", point.transform.translate.x }, { "y", point.transform.translate.y }, { "z", point.transform.translate.z }
			};
			pointJson["secondsToNextPoint"] = point.secondsToNextPoint;
			points.push_back(std::move(pointJson));
		}
		json_[name] = std::move(points);
	}
}

void QFE::JsonArchive::Process(const std::string& name, MATH::Matrix4x4& value) {
    if (isLoading_) {
        // デシリアライズ
        if (json_.contains(name)) {
            for (int row = 0; row < 4; ++row) {
                for (int col = 0; col < 4; ++col) {
                    value.Set(row, col, json_[name][row][col].get<float>());
                }
            }
        }
    } else {
        // シリアライズ
        for (int row = 0; row < 4; ++row) {
            for (int col = 0; col < 4; ++col) {
                json_[name][row][col] = value.Get(row, col);
            }
        }
	}
}

void QFE::JsonArchive::Process(const std::string& name, MATH::Bit32& value) {
    if (isLoading_) {
        // デシリアライズ
        if (json_.contains(name)) value.value = json_[name].get<uint32_t>();
    } else {
        // シリアライズ
        json_[name] = value.value;
	}
}

void QFE::JsonArchive::Process(const std::string& name, EntityReference& value) {
	if (isLoading_) {
		if (json_.contains(name)) value.uuid = json_[name].get<std::string>();
	} else {
		json_[name] = value.uuid;
	}
}

void QFE::JsonArchive::Process(const std::string& name, nlohmann::json& value) {
	if (isLoading_) {
		if (json_.contains(name)) value = json_[name];
	} else {
		json_[name] = value;
	}
}
