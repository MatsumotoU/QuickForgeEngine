#pragma once

#include "animation/AnimationComponent.h"
#include "BGMComponent.h"
#include "CameraComponent.h"
#include "CollisionTriggerComponent.h"
#include "EventComponent.h"
#include "InputMovementComponent.h"
#include "MaterialComponent.h"
#include "ModelRenderComponent.h"
#include "ObjectInfoComponent.h"
#include "ParentComponent.h"
#include "PhysicsComponent.h"
#include "SceneChangeComponent.h"
#include "ScriptComponent.h"
#include "SEComponent.h"
#include "ShaderComponent.h"
#include "SkyBoxComponent.h"
#include "SphereColliderComponent.h"
#include "SplineMoveToPointComponent.h"
#include "SplineMovementComponent.h"
#include "SpriteRenderComponent.h"
#include "TransformComponent.h"
#include "VertexComponent.h"
#include "core/design-patterns/component/StaticComponentRegistry.h"

namespace QFE {
	/// @brief エンジンが標準提供するコンポーネントを登録する。
	inline bool RegisterEngineComponents(StaticComponentRegistry& registry) {
		return
			registry.RegisterComponent<SCENE::TransformComponent>("TransformComponent") &&
			registry.RegisterComponent<SCENE::ParentComponent>("ParentComponent") &&
			registry.RegisterComponent<SCENE::ObjectInfoComponent>("ObjectInfoComponent") &&
			registry.RegisterComponent<SCENE::VertexComponent>("VertexComponent") &&
			registry.RegisterComponent<SCENE::MaterialComponent>("MaterialComponent") &&
			registry.RegisterComponent<SCENE::ModelRenderComponent>("ModelRenderComponent") &&
			registry.RegisterComponent<SCENE::ShaderComponent>("ShaderComponent") &&
			registry.RegisterComponent<SCENE::SkyBoxComponent>("SkyBoxComponent") &&
			registry.RegisterComponent<SCENE::SpriteRenderComponent>("SpriteRenderComponent") &&
			registry.RegisterComponent<SCENE::ScriptComponent>("ScriptComponent") &&
			registry.RegisterComponent<SCENE::CameraComponent>("CameraComponent") &&
			registry.RegisterComponent<SCENE::SphereColliderComponent>("SphereColliderComponent") &&
			registry.RegisterComponent<SCENE::AnimationComponent>("AnimationComponent") &&
			registry.RegisterComponent<COMPONENTS::PhysicsComponent>("PhysicsComponent") &&
			registry.RegisterComponent<COMPONENTS::InputMovementComponent>("InputMovementComponent") &&
			registry.RegisterComponent<SCENE::SplineMovementComponent>("SplineMovementComponent") &&
			registry.RegisterComponent<SCENE::SplineMoveToPointComponent>("SplineMoveToPointComponent") &&
			registry.RegisterComponent<SCENE::EventComponent>("EventComponent") &&
			registry.RegisterComponent<SCENE::CollisionTriggerComponent>("CollisionTriggerComponent") &&
			registry.RegisterComponent<SCENE::BGMComponent>("BGMComponent") &&
			registry.RegisterComponent<SCENE::SEComponent>("SEComponent") &&
			registry.RegisterComponent<SCENE::SceneChangeComponent>("SceneChangeComponent");
	}
}
