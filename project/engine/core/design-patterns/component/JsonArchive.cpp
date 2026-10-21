#include "JsonArchive.h"
#include "components/SplineControlPoint.h"

#include <algorithm>
#include <utility>

namespace {
	template <typename T>
	void SetField(nlohmann::json& json, const std::string& name, T&& value) {
		// jsonがnullの場合はオブジェクトとして初期化する
		if (json.is_null()) {
			json = nlohmann::json::object();
		}

		// emplace()は既存キーを置き換えないため、既存フィールドは代入する。
		auto field = json.find(name);
		if (field == json.end()) {
			json.emplace(name, std::forward<T>(value));
		} else {
			field.value() = std::forward<T>(value);
		}
	}

	nlohmann::json& GetOrCreateObject(nlohmann::json& json, const std::string& name) {
		auto field = json.find(name);
		if (field == json.end()) {
			field = json.emplace(name, nlohmann::json::object()).first;
		}
		return field.value();
	}
}

void QFE::JsonArchive::Process(const std::string& name, bool& value) {
    if (isLoading_) {
        // デシリアライズ
        if (json_.contains(name)) value = json_.at(name).get<bool>();
    } else {
        // シリアライズ
        SetField(json_, name, value);
	}
}

void QFE::JsonArchive::Process(const std::string& name, float& value) {
    if (isLoading_) {
		// デシリアライズ
        if (json_.contains(name)) value = json_.at(name).get<float>();
    } else {
		// シリアライズ
        SetField(json_, name, value);
    }
}

void QFE::JsonArchive::Process(const std::string& name, int32_t& value) {
    if (isLoading_) {
		// デシリアライズ
        if (json_.contains(name)) value = json_.at(name).get<int32_t>();
    } else {
		// シリアライズ
        SetField(json_, name, value);
    }
}

void QFE::JsonArchive::Process(const std::string& name, uint32_t& value) {
    if (isLoading_) {
        // デシリアライズ
        if (json_.contains(name)) value = json_.at(name).get<uint32_t>();
    } else {
        // シリアライズ
        SetField(json_, name, value);
	}
}

void QFE::JsonArchive::Process(const std::string& name, std::string& value) {
    if (isLoading_) {
		// デシリアライズ
        if (json_.contains(name)) value = json_.at(name).get<std::string>();
    } else {
		// シリアライズ
        SetField(json_, name, value);
    }
}

void QFE::JsonArchive::Process(const std::string& name, std::vector<std::string>& value) {
	if (isLoading_) {
		const auto field = json_.find(name);
		if (field == json_.end() || !field->is_array()) return;

		std::vector<std::string> loadedValues;
		loadedValues.reserve(field->size());
		for (const nlohmann::json& item : *field) {
			if (item.is_string()) loadedValues.push_back(item.get<std::string>());
		}
		value = std::move(loadedValues);
	} else {
		SetField(json_, name, value);
	}
}

void QFE::JsonArchive::Process(const std::string& name, MATH::Vector2& value) {
    if (isLoading_) {
        // デシリアライズ
        if (json_.contains(name)) {
            const auto& vectorJson = json_.at(name);
            value.x = vectorJson.at("x").get<float>();
            value.y = vectorJson.at("y").get<float>();
        }
    } else {
        // シリアライズ
		auto& vectorJson = GetOrCreateObject(json_, name);
		SetField(vectorJson, "x", value.x);
		SetField(vectorJson, "y", value.y);
	}
}

void QFE::JsonArchive::Process(const std::string& name, MATH::Vector3& value) {
    if (isLoading_) {
        // デシリアライズ
        if (json_.contains(name)) {
            const auto& vectorJson = json_.at(name);
            value.x = vectorJson.at("x").get<float>();
            value.y = vectorJson.at("y").get<float>();
            value.z = vectorJson.at("z").get<float>();
        }
    } else {
        // シリアライズ
        auto& vectorJson = GetOrCreateObject(json_, name);
        SetField(vectorJson, "x", value.x);
        SetField(vectorJson, "y", value.y);
        SetField(vectorJson, "z", value.z);
    }
}

void QFE::JsonArchive::Process(const std::string& name, MATH::Vector4& value) {
    if (isLoading_) {
        // デシリアライズ
        if (json_.contains(name)) {
            const auto& vectorJson = json_.at(name);
            value.x = vectorJson.at("x").get<float>();
            value.y = vectorJson.at("y").get<float>();
            value.z = vectorJson.at("z").get<float>();
            value.w = vectorJson.at("w").get<float>();
        }
    } else {
        // シリアライズ
        auto& vectorJson = GetOrCreateObject(json_, name);
        SetField(vectorJson, "x", value.x);
        SetField(vectorJson, "y", value.y);
        SetField(vectorJson, "z", value.z);
        SetField(vectorJson, "w", value.w);
    }
}

void QFE::JsonArchive::Process(const std::string& name, MATH::EulerTransform& value) {
    if (isLoading_) {
        // デシリアライズ
        if (json_.contains(name)) {
			const auto& transformJson = json_.at(name);
			const auto& scaleJson = transformJson.at("scale");
			const auto& rotateJson = transformJson.at("rotate");
			const auto& translateJson = transformJson.at("translate");
			scaleJson.at("x").get_to(value.scale.x);
			scaleJson.at("y").get_to(value.scale.y);
			scaleJson.at("z").get_to(value.scale.z);
			rotateJson.at("x").get_to(value.rotate.x);
			rotateJson.at("y").get_to(value.rotate.y);
			rotateJson.at("z").get_to(value.rotate.z);
			translateJson.at("x").get_to(value.translate.x);
			translateJson.at("y").get_to(value.translate.y);
			translateJson.at("z").get_to(value.translate.z);
        }
    } else {
        // シリアライズ
		auto& transformJson = GetOrCreateObject(json_, name);
		auto& scaleJson = GetOrCreateObject(transformJson, "scale");
		auto& rotateJson = GetOrCreateObject(transformJson, "rotate");
		auto& translateJson = GetOrCreateObject(transformJson, "translate");
		SetField(scaleJson, "x", value.scale.x);
		SetField(scaleJson, "y", value.scale.y);
		SetField(scaleJson, "z", value.scale.z);
		SetField(rotateJson, "x", value.rotate.x);
		SetField(rotateJson, "y", value.rotate.y);
		SetField(rotateJson, "z", value.rotate.z);
		SetField(translateJson, "x", value.translate.x);
		SetField(translateJson, "y", value.translate.y);
		SetField(translateJson, "z", value.translate.z);
	}
}

void QFE::JsonArchive::Process(const std::string& name, std::vector<MATH::EulerTransform>& value) {
    if (isLoading_) {
        const auto field = json_.find(name);
        if (field == json_.end() || !field->is_array()) {
            return;
        }

        std::vector<MATH::EulerTransform> loadedPoints;
        loadedPoints.reserve(field->size());
        for (const nlohmann::json& pointJson : *field) {
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
        SetField(json_, name, std::move(points));
    }
}

void QFE::JsonArchive::Process(const std::string& name, std::vector<SCENE::SplineControlPoint>& value) {
	if (isLoading_) {
		const auto field = json_.find(name);
		if (field == json_.end() || !field->is_array()) {
			return;
		}

		// 旧形式のスプラインには全体のspeedしかないため、その値を各区間の初期値として引き継ぐ。
		const float legacySecondsToNextPoint = json_.value("speed", 1.0f);
		std::vector<SCENE::SplineControlPoint> loadedPoints;
		loadedPoints.reserve(field->size());
		for (const nlohmann::json& pointJson : *field) {
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
			auto& transformJson = GetOrCreateObject(pointJson, "transform");
			SetField(transformJson, "scale", nlohmann::json{
				{ "x", point.transform.scale.x }, { "y", point.transform.scale.y }, { "z", point.transform.scale.z }
			});
			SetField(transformJson, "rotate", nlohmann::json{
				{ "x", point.transform.rotate.x }, { "y", point.transform.rotate.y }, { "z", point.transform.rotate.z }
			});
			SetField(transformJson, "translate", nlohmann::json{
				{ "x", point.transform.translate.x }, { "y", point.transform.translate.y }, { "z", point.transform.translate.z }
			});
			SetField(pointJson, "secondsToNextPoint", point.secondsToNextPoint);
			points.push_back(std::move(pointJson));
		}
		SetField(json_, name, std::move(points));
	}
}

void QFE::JsonArchive::ProcessReflectedArray(
	const std::string& name,
	const std::string&,
	std::size_t size,
	const ResizeReflectedArray& resize,
	const EraseReflectedArrayItem&,
	const ReflectArrayItem& reflect) {
	if (isLoading_) {
		const auto field = json_.find(name);
		if (field == json_.end() || !field->is_array()) {
			return;
		}

		resize(field->size());
		for (std::size_t index = 0; index < field->size(); ++index) {
			nlohmann::json& item = (*field)[index];
			if (!item.is_object()) {
				continue;
			}
			JsonArchive itemArchive(item, true);
			reflect(index, itemArchive);
		}
		return;
	}

	nlohmann::json serializedValues = nlohmann::json::array();
	for (std::size_t index = 0; index < size; ++index) {
		nlohmann::json item = nlohmann::json::object();
		JsonArchive itemArchive(item, false);
		reflect(index, itemArchive);
		serializedValues.push_back(std::move(item));
	}
	SetField(json_, name, std::move(serializedValues));
}

void QFE::JsonArchive::Process(const std::string& name, MATH::Matrix4x4& value) {
    if (isLoading_) {
        // デシリアライズ
        if (json_.contains(name)) {
			const auto& matrixJson = json_.at(name);
            for (int row = 0; row < 4; ++row) {
                for (int col = 0; col < 4; ++col) {
                    value.Set(row, col,
                        matrixJson.at(static_cast<nlohmann::json::size_type>(row))
                            .at(static_cast<nlohmann::json::size_type>(col)).get<float>());
                }
            }
        }
    } else {
        // シリアライズ
		nlohmann::json matrixJson = nlohmann::json::array();
        for (int row = 0; row < 4; ++row) {
			nlohmann::json rowJson = nlohmann::json::array();
            for (int col = 0; col < 4; ++col) {
				rowJson.push_back(value.Get(row, col));
            }
			matrixJson.push_back(std::move(rowJson));
        }
		SetField(json_, name, std::move(matrixJson));
	}
}

void QFE::JsonArchive::Process(const std::string& name, MATH::Bit32& value) {
    if (isLoading_) {
        // デシリアライズ
        if (json_.contains(name)) value.value = json_.at(name).get<uint32_t>();
    } else {
        // シリアライズ
        SetField(json_, name, value.value);
	}
}

void QFE::JsonArchive::Process(const std::string& name, EntityReference& value) {
	if (isLoading_) {
		if (json_.contains(name)) value.uuid = json_.at(name).get<std::string>();
	} else {
		SetField(json_, name, value.uuid);
	}
}

void QFE::JsonArchive::Process(const std::string& name, nlohmann::json& value) {
	if (isLoading_) {
		if (json_.contains(name)) value = json_.at(name);
	} else {
		SetField(json_, name, value);
	}
}
