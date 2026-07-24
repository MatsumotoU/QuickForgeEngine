#pragma once
#define NOMINMAX
#include <windows.h>
#include "script/ScriptFunctionList.h"
#include "design-patterns/component/DynamicComponent.h"

void MoveZ(uint32_t entityId, float dt, QFE::IEntityManager* entityManager);

extern "C" {
    __declspec(dllexport) size_t GetManifest(QFE::SCRIPT::ScriptFunctionInfo** outArray);
	__declspec(dllexport) bool QFE_GetComponentManifest(QFE::PluginComponentManifest* outManifest);
}
