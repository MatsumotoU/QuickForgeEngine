#pragma once
#include <stdint.h>
#include <mono/jit/jit.h>

#include "engine/include/core/Math/Vector/Vector3.h"
#include "engine/include/core/Entity/EntityManager.h"

namespace QFE::CsharpOnQFELinker {
    // Transform
    extern void GetTransformTranslate(uint32_t entityId, Vector3* outTranslate,EntityManager* entityManager);
    extern void SetTransformTranslate(uint32_t entityId, Vector3* inTranslate, EntityManager* entityManager);
    extern void GetTransformRotate(uint32_t entityId, Vector3* outRotate, EntityManager* entityManager);
    extern void SetTransformRotate(uint32_t entityId, Vector3* inRotate, EntityManager* entityManager);
    extern void GetTransformScale(uint32_t entityId, Vector3* outScale, EntityManager* entityManager);
    extern void SetTransformScale(uint32_t entityId, Vector3* inScale, EntityManager* entityManager);
    extern void Translate(uint32_t entityId, Vector3* translation, EntityManager* entityManager);
    extern void Rotate(uint32_t entityId, Vector3* eulerAngles, EntityManager* entityManager);

    // Time
	extern float GetDeltaTime();

    // Input
	extern bool IsKeyTrigger(MonoString* actionName);
	extern bool IsKeyPress(MonoString* actionName);
	extern bool IsKeyRelease(MonoString* actionName);

	// SceneObject
	extern uint32_t CreateEntity(MonoString* className);

	// debug
    extern void Native_Debug_Log(MonoString* message);
}
