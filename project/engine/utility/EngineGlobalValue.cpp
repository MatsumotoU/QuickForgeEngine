/**
 * @file EngineGlobalValue.cpp
 * @brief エンジン全体で使用される共通変数の実体定義
 */

#include "engine/include/core/EngineGlobalValue.h"

namespace QFE::EngineGlobalValue {
    uint32_t windowWidth = 0;
    uint32_t windowHeight = 0;
    float deltaTime = 0.0f;
    float fps = 0.0f;
}
