#pragma once
#include <stdint.h>
#include <mono/jit/jit.h>

#include "engine/include/core/Math/Vector/Vector3.h"
#include "engine/include/core/Math/Vector/Vector2.h"
#include "engine/include/core/Entity/EntityManager.h"

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
	extern void GetKeyMoveDir(Vector2* outDir);
	extern bool IsKeyTrigger(MonoString* actionName);
	extern bool IsKeyPress(MonoString* actionName);
	extern bool IsKeyRelease(MonoString* actionName);

    // Mouse
    extern bool GetMousePress(int8_t button);
    extern bool GetMouseTrigger(int8_t button);
    extern bool GetMouseRelease(int8_t button);
    extern void GetMouseScreenPos(Vector2* outPos);
    extern void GetMouseMoveDir(Vector2* outDir);
    extern float GetMouseWheelDir();

	// Audio
	extern uint32_t LoadSound(MonoString* soundName);
	extern uint32_t PlayQFESound(uint32_t soundHandle, bool isLoop, float volume);
	extern void StopSound(uint32_t playHandle);

	// SceneObject
	extern uint32_t GetEntity(MonoString* entityName);
	extern uint32_t CreateEntity(MonoString* entityName);
	extern void LoadScene(MonoString* sceneName);
    extern void ChangeModel(uint32_t entityId, MonoString* modelName);
    extern void ChangeMesh(uint32_t entityId, MonoString* meshName);

	// debug
    extern void Native_Debug_Log(MonoString* message);
}
