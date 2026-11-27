#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>
#include <imgui/imgui.h>
#include <nlohmann/json.hpp>
#include "math/matrix/Matrix4x4.h"
#include "framework/input/InputFrameWork.h"

namespace QFE {
	class EntityManager;
}

namespace QFE::EDITOR {
	/// @brief ゲーム固有コンポーネントのInspector描画関数を保持する。
	class ComponentInspectorRegistry final {
	public:
		using InspectorFunction = void (*)(EntityManager&, uint32_t, const std::vector<FRAMEWORK::GameInputAction>*);
		using SceneOverlayFunction = void (*)(EntityManager&, const MATH::Matrix4x4&, const ImVec2&, const ImVec2&);
		using CollisionActionInspector = void (*)(nlohmann::json&);
		bool RegisterCollisionAction(const std::string& name, CollisionActionInspector inspector) {
			return !name.empty() && inspector && collisionActions_.emplace(name, inspector).second;
		}
		const auto& GetCollisionActions() const { return collisionActions_; }
		bool DrawCollisionAction(const std::string& name, nlohmann::json& action) const {
			const auto found = collisionActions_.find(name);
			if (found == collisionActions_.end()) return false;
			found->second(action);
			return true;
		}

		// ゲーム固有の可視化を登録し、共通エディターからゲーム側の型を参照しない。
		void RegisterSceneOverlay(SceneOverlayFunction overlay) {
			if (overlay) overlays_.push_back(overlay);
		}
		void DrawSceneOverlays(EntityManager& entities, const MATH::Matrix4x4& viewProjection,
			const ImVec2& position, const ImVec2& size) const {
			for (const auto overlay : overlays_) overlay(entities, viewProjection, position, size);
		}

		bool Register(const std::string& componentKey, InspectorFunction inspector) {
			if (componentKey.empty() || inspector == nullptr || inspectors_.contains(componentKey)) {
				return false;
			}
			inspectors_.emplace(componentKey, inspector);
			return true;
		}

		bool Draw(const std::string& componentKey, EntityManager& entityManager, uint32_t entityId,
			const std::vector<FRAMEWORK::GameInputAction>* gameInputActions) const {
			const auto inspector = inspectors_.find(componentKey);
			if (inspector == inspectors_.end()) {
				return false;
			}
			inspector->second(entityManager, entityId, gameInputActions);
			return true;
		}

	private:
		std::unordered_map<std::string, InspectorFunction> inspectors_;
		std::vector<SceneOverlayFunction> overlays_;
		std::unordered_map<std::string, CollisionActionInspector> collisionActions_;
	};
}
