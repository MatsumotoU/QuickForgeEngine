#pragma once
#include <stdint.h>
#include <mono/jit/jit.h>

#include "engine/include/core/Math/Vector/Vector3.h"

namespace QFE::CsharpOnQFELinker {
    // Transform
    extern void GetTransformTranslate(uint32_t entityId, Vector3* outTranslate);
    extern void SetTransformTranslate(uint32_t entityId, Vector3* inTranslate);
    extern void GetTransformRotate(uint32_t entityId, Vector3* outRotate);
    extern void SetTransformRotate(uint32_t entityId, Vector3* inRotate);
    extern void GetTransformScale(uint32_t entityId, Vector3* outScale);
    extern void SetTransformScale(uint32_t entityId, Vector3* inScale);
    extern void Translate(uint32_t entityId, Vector3* translation);
    extern void Rotate(uint32_t entityId, Vector3* eulerAngles);

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
