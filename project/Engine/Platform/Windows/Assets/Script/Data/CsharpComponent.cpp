#include "CsharpComponent.h"

nlohmann::json CsharpComponent::Serialize() const {
    return nlohmann::json();
}

void CsharpComponent::Deserialize(const nlohmann::json& json) {
    json;
}
