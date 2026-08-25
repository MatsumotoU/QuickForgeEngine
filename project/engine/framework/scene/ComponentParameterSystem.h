#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "design-patterns/component/EntityReference.h"
#include "../../../externals/nlohmann/json.hpp"

namespace QFE {
	class EntityManager;
}

namespace QFE::FRAMEWORK {
	enum class ParameterTarget : uint32_t {
		Self,
		Other,
		EntityReference
	};

	enum class ParameterOperation : uint32_t {
		Set,
		Add,
		Multiply,
		Toggle,
		SetBits,
		ClearBits
	};

	struct ComponentParameterRequest {
		ParameterTarget target = ParameterTarget::Self;
		EntityReference entity;
		std::string componentName;
		std::string propertyPath;
		ParameterOperation operation = ParameterOperation::Set;
		nlohmann::json value;
	};

	struct ComponentParameterCommand {
		uint32_t selfEntityId = UINT32_MAX;
		uint32_t otherEntityId = UINT32_MAX;
		ComponentParameterRequest request;
	};

	/// @brief UUID・コンポーネント名・プロパティパスを解決して値を変更する。
	bool ApplyComponentParameter(
		EntityManager& entityManager,
		uint32_t selfEntityId,
		uint32_t otherEntityId,
		const ComponentParameterRequest& request);

	/// @brief キューに積まれた操作を順番に同一フレーム内で反映する。
	void ApplyComponentParameterRequests(
		EntityManager& entityManager,
		const std::vector<ComponentParameterRequest>& requests,
		uint32_t selfEntityId,
		uint32_t otherEntityId = UINT32_MAX);
	void ApplyComponentParameterCommands(
		EntityManager& entityManager,
		const std::vector<ComponentParameterCommand>& commands);

	ParameterTarget ParameterTargetFromString(const std::string& value);
	ParameterOperation ParameterOperationFromString(const std::string& value);
	const char* ToString(ParameterTarget value);
	const char* ToString(ParameterOperation value);
}
