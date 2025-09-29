#pragma once
#include <nlohmann/json.hpp>
class ComponentData {
public:
	virtual ~ComponentData() = default;
	virtual nlohmann::json Serialize() const = 0;
};