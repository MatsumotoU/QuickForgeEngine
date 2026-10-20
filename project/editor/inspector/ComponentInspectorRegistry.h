#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>

namespace QFE {
	class EntityManager;
}

namespace QFE::EDITOR {
	/// @brief ゲーム固有コンポーネントのInspector描画関数を保持する。
	class ComponentInspectorRegistry final {
	public:
		using InspectorFunction = void (*)(EntityManager&, uint32_t);

		bool Register(const std::string& componentKey, InspectorFunction inspector) {
			if (componentKey.empty() || inspector == nullptr || inspectors_.contains(componentKey)) {
				return false;
			}
			inspectors_.emplace(componentKey, inspector);
			return true;
		}

		bool Draw(const std::string& componentKey, EntityManager& entityManager, uint32_t entityId) const {
			const auto inspector = inspectors_.find(componentKey);
			if (inspector == inspectors_.end()) {
				return false;
			}
			inspector->second(entityManager, entityId);
			return true;
		}

	private:
		std::unordered_map<std::string, InspectorFunction> inspectors_;
	};
}
