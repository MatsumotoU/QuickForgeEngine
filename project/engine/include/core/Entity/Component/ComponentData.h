#pragma once
#include <nlohmann/json.hpp>
#include <string>

namespace QFE {

    class ComponentData {
    public:
        virtual ~ComponentData() = default;
        virtual nlohmann::json Serialize() const = 0;
        virtual void Deserialize(const nlohmann::json& json) = 0;
        virtual std::string GetTypeName() const = 0;
    };

}
