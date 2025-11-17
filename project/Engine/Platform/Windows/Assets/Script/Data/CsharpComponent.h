#pragma once
#include "Core/Entity/Component/ComponentData.h"
#include <mono/jit/jit.h>
#include <Mono/metadata/object.h>
#include <unordered_map>

class CsharpComponent :public ComponentData {
	CsharpComponent() = default;
	~CsharpComponent() override = default;
	nlohmann::json Serialize() const override;
	void Deserialize(const nlohmann::json& json) override;
	std::string GetTypeName() const override { return "CsharpComponent"; }

	MonoObject* monoObject_ = nullptr;
	std::unordered_map<std::string, MonoMethod*> methodCache_;	
};

