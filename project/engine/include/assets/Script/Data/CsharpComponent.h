#pragma once
#include "engine/include/core/Entity/Component/ComponentData.h"
#include <mono/jit/jit.h>
#include <Mono/metadata/object.h>
#include <vector>
namespace QFE {
	struct CsharpHandle {
		std::string className_;
		uint32_t scriptIndex_;
	};

	class CsharpComponent :public ComponentData {
	public:
		CsharpComponent() = default;
		~CsharpComponent() override = default;
		nlohmann::json Serialize() const override;
		void Deserialize(const nlohmann::json& json) override;
		std::string GetTypeName() const override { return "CsharpComponent"; }

		std::vector<CsharpHandle> csharpHandles_;
	};
}
