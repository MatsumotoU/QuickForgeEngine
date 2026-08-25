#include "ComponentParameterSystem.h"

#include "design-patterns/EntityManager.h"
#include "components/TransformHierarchy.h"

#include <limits>

namespace {
	using Json = nlohmann::json;

	Json* FindProperty(Json& root, const std::string& path) {
		Json* current = &root;
		size_t begin = 0;
		while (begin <= path.size()) {
			const size_t end = path.find('.', begin);
			const std::string key = path.substr(begin, end - begin);
			if (key.empty() || !current->is_object()) return nullptr;
			auto iterator = current->find(key);
			if (iterator == current->end()) return nullptr;
			current = &(*iterator);
			if (end == std::string::npos) return current;
			begin = end + 1;
		}
		return nullptr;
	}

	bool ApplyNumericOperation(Json& target, const Json& operand,
		QFE::FRAMEWORK::ParameterOperation operation) {
		if (!target.is_number() || !operand.is_number()) return false;
		const double lhs = target.get<double>();
		const double rhs = operand.get<double>();
		switch (operation) {
		case QFE::FRAMEWORK::ParameterOperation::Add:
			target = lhs + rhs;
			return true;
		case QFE::FRAMEWORK::ParameterOperation::Multiply:
			target = lhs * rhs;
			return true;
		case QFE::FRAMEWORK::ParameterOperation::SetBits:
			target = target.get<uint32_t>() | operand.get<uint32_t>();
			return true;
		case QFE::FRAMEWORK::ParameterOperation::ClearBits:
			target = target.get<uint32_t>() & ~operand.get<uint32_t>();
			return true;
		default:
			return false;
		}
	}

	bool ApplyOperation(Json& target, const Json& operand,
		QFE::FRAMEWORK::ParameterOperation operation) {
		if (operation == QFE::FRAMEWORK::ParameterOperation::Set) {
			target = operand;
			return true;
		}
		if (operation == QFE::FRAMEWORK::ParameterOperation::Toggle) {
			if (!target.is_boolean()) return false;
			target = !target.get<bool>();
			return true;
		}
		if (ApplyNumericOperation(target, operand, operation)) return true;

		if (target.is_object() && operand.is_object() &&
			(operation == QFE::FRAMEWORK::ParameterOperation::Add ||
				operation == QFE::FRAMEWORK::ParameterOperation::Multiply)) {
			bool changed = false;
			for (auto iterator = target.begin(); iterator != target.end(); ++iterator) {
				if (operand.contains(iterator.key())) {
					changed |= ApplyOperation(iterator.value(), operand[iterator.key()], operation);
				}
			}
			return changed;
		}
		return false;
	}
}

bool QFE::FRAMEWORK::ApplyComponentParameter(
	EntityManager& entityManager, uint32_t selfEntityId, uint32_t otherEntityId,
	const ComponentParameterRequest& request) {
	uint32_t targetEntityId = selfEntityId;
	switch (request.target) {
	case ParameterTarget::Other:
		if (otherEntityId == UINT32_MAX) return false;
		targetEntityId = otherEntityId;
		break;
	case ParameterTarget::EntityReference:
		if (!SCENE::TryGetEntityIdByUuid(entityManager, request.entity.uuid, targetEntityId)) return false;
		break;
	case ParameterTarget::Self:
	default:
		break;
	}

	if (!entityManager.IsActiveEntity(targetEntityId) || request.componentName.empty() ||
		request.propertyPath.empty() ||
		entityManager.GetComponentRaw(targetEntityId, request.componentName.c_str()) == nullptr) {
		return false;
	}

	nlohmann::json componentJson =
		entityManager.SerializeComponent(targetEntityId, request.componentName);
	nlohmann::json* property = FindProperty(componentJson, request.propertyPath);
	if (property == nullptr || !ApplyOperation(*property, request.value, request.operation)) return false;

	entityManager.DeserializeEntityComponents(targetEntityId, nlohmann::json::object({
		{ request.componentName, std::move(componentJson) }
	}));
	return true;
}

void QFE::FRAMEWORK::ApplyComponentParameterRequests(
	EntityManager& entityManager, const std::vector<ComponentParameterRequest>& requests,
	uint32_t selfEntityId, uint32_t otherEntityId) {
	for (const ComponentParameterRequest& request : requests) {
		ApplyComponentParameter(entityManager, selfEntityId, otherEntityId, request);
	}
}

void QFE::FRAMEWORK::ApplyComponentParameterCommands(
	EntityManager& entityManager, const std::vector<ComponentParameterCommand>& commands) {
	for (const ComponentParameterCommand& command : commands) {
		ApplyComponentParameter(
			entityManager, command.selfEntityId, command.otherEntityId, command.request);
	}
}

QFE::FRAMEWORK::ParameterTarget QFE::FRAMEWORK::ParameterTargetFromString(const std::string& value) {
	if (value == "Other") return ParameterTarget::Other;
	if (value == "EntityReference" || value == "Entity") return ParameterTarget::EntityReference;
	return ParameterTarget::Self;
}

QFE::FRAMEWORK::ParameterOperation QFE::FRAMEWORK::ParameterOperationFromString(const std::string& value) {
	if (value == "Add") return ParameterOperation::Add;
	if (value == "Multiply") return ParameterOperation::Multiply;
	if (value == "Toggle") return ParameterOperation::Toggle;
	if (value == "SetBits") return ParameterOperation::SetBits;
	if (value == "ClearBits") return ParameterOperation::ClearBits;
	return ParameterOperation::Set;
}

const char* QFE::FRAMEWORK::ToString(ParameterTarget value) {
	switch (value) {
	case ParameterTarget::Other: return "Other";
	case ParameterTarget::EntityReference: return "EntityReference";
	default: return "Self";
	}
}

const char* QFE::FRAMEWORK::ToString(ParameterOperation value) {
	switch (value) {
	case ParameterOperation::Add: return "Add";
	case ParameterOperation::Multiply: return "Multiply";
	case ParameterOperation::Toggle: return "Toggle";
	case ParameterOperation::SetBits: return "SetBits";
	case ParameterOperation::ClearBits: return "ClearBits";
	default: return "Set";
	}
}
