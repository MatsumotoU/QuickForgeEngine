#include "ParentSerializer.h"

nlohmann::json ParentSerializer::Serialize(const ParentComponent& parent) {
	nlohmann::json j;
	j["parentID"] = parent.parentEntityId;
	return j;
}

void ParentSerializer::Deserialize(ParentComponent& parent, const nlohmann::json& j) {
	if (j.contains("parentID")) {
		parent.parentEntityId = j["parentID"].get<uint32_t>();
	}
}
