#pragma once

#include "animation/AnimationComponent.h"
#include "BGMComponent.h"
#include "CameraComponent.h"
#include "DirectionalLightComponent.h"
#include "CollisionTriggerComponent.h"
#include "EventComponent.h"
#include "InputMovementComponent.h"
#include "MaterialComponent.h"
#include "ModelRenderComponent.h"
#include "ObjectInfoComponent.h"
#include "ParentComponent.h"
#include "ParticleEmitterComponent.h"
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
			registry.RegisterComponent<SCENE::TransformComponent>("TransformComponent", "Core") &&
			registry.RegisterComponent<SCENE::ParentComponent>("ParentComponent", "Core") &&
			registry.RegisterComponent<SCENE::ObjectInfoComponent>("ObjectInfoComponent", "Core") &&
			registry.RegisterComponent<SCENE::VertexComponent>("VertexComponent", "Rendering") &&
			registry.RegisterComponent<SCENE::MaterialComponent>("MaterialComponent", "Rendering") &&
			registry.RegisterComponent<SCENE::ModelRenderComponent>("ModelRenderComponent", "Rendering") &&
			registry.RegisterComponent<SCENE::ShaderComponent>("ShaderComponent", "Rendering") &&
			registry.RegisterComponent<SCENE::SkyBoxComponent>("SkyBoxComponent", "Rendering") &&
			registry.RegisterComponent<SCENE::SpriteRenderComponent>("SpriteRenderComponent", "Rendering") &&
			registry.RegisterComponent<SCENE::CameraComponent>("CameraComponent", "Rendering") &&
			registry.RegisterComponent<SCENE::DirectionalLightComponent>("DirectionalLightComponent", "Rendering") &&
			registry.RegisterComponent<SCENE::AnimationComponent>("AnimationComponent", "Rendering") &&
			registry.RegisterComponent<SCENE::ParticleEmitterComponent>("ParticleEmitterComponent", "Rendering") &&
			registry.RegisterComponent<SCENE::SphereColliderComponent>("SphereColliderComponent", "Physics") &&
			registry.RegisterComponent<COMPONENTS::PhysicsComponent>("PhysicsComponent", "Physics") &&
			registry.RegisterComponent<SCENE::CollisionTriggerComponent>("CollisionTriggerComponent", "Physics") &&
			registry.RegisterComponent<SCENE::BGMComponent>("BGMComponent", "Audio") &&
			registry.RegisterComponent<SCENE::SEComponent>("SEComponent", "Audio") &&
			registry.RegisterComponent<SCENE::ScriptComponent>("ScriptComponent", "Behavior") &&
			registry.RegisterComponent<COMPONENTS::InputMovementComponent>("InputMovementComponent", "Behavior") &&
			registry.RegisterComponent<SCENE::SplineMovementComponent>("SplineMovementComponent", "Behavior") &&
			registry.RegisterComponent<SCENE::SplineMoveToPointComponent>("SplineMoveToPointComponent", "Behavior") &&
			registry.RegisterComponent<SCENE::EventComponent>("EventComponent", "Behavior") &&
			registry.RegisterComponent<SCENE::SceneChangeComponent>("SceneChangeComponent", "Behavior");
	}
}
