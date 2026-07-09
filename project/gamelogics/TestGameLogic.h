#pragma once
#define NOMINMAX
#include <windows.h>
#include "script/ScriptFunctionList.h"

void Script_PlayerMove(uint32_t entityId, float dt,QFE::EntityManager* entityManager);
void Script_EnemyAI(uint32_t entityId, float dt, QFE::EntityManager* entityManager);

extern "C" {
    __declspec(dllexport) size_t GetManifest(QFE::SCRIPT::ScriptFunctionInfo** outArray);
}