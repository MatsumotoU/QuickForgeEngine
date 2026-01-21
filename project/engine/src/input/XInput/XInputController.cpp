/**
 * @file XInputController.cpp
 * @brief XInput繧剃ｽｿ逕ｨ縺励◆繧ｲ繝ｼ繝繝代ャ繝牙・蜉帷ｮ｡逅・け繝ｩ繧ｹ縺ｮ螳溯｣・
 */

#include "engine/include/input/XInput/XInputController.h"
#pragma comment(lib, "Xinput.lib")

#ifdef QFE_OPTIMIZE_OFF
#include "engine/include/utility/DebugTool/DebugLog/MyDebugLog.h"
#endif // QFE_OPTIMIZE_OFF

#include <assert.h>
#include <cmath>

/** @brief 繧ｳ繝ｳ繧ｹ繝医Λ繧ｯ繧ｿ */
XInputController::XInputController() {
    stickDeadZone_ = 7849.0f;
}

/** @brief 譖ｴ譁ｰ */
void XInputController::Update() {
    prevGamepadStates = gamepadStates;

	for (DWORD i = 0; i < XUSER_MAX_COUNT; i++) {
		DWORD dwResult = XInputGetState(i, &gamepadStates[i].state);

        if (dwResult == ERROR_SUCCESS) {
            if (!gamepadStates[i].isConnected) {
                gamepadStates[i].isConnected = true;
        #ifdef QFE_OPTIMIZE_OFF
                DebugLog(std::format("Connected Controller! paletNumber: {}", gamepadStates[i].state.dwPacketNumber));

                // 讖溯・諠・ｱ繧貞叙蠕・
                XINPUT_CAPABILITIES cap;
                if (XInputGetCapabilities(i, 0, &cap) == ERROR_SUCCESS) {
                    DebugLog(std::format("Type: {}", static_cast<int>(cap.Type)));
                    DebugLog(std::format("SubType: {}", static_cast<int>(cap.SubType)));
                    DebugLog(std::format("Flags: 0x{:X}", static_cast<unsigned int>(cap.Flags)));
                } else {
                    DebugLog("Failed to get capabilities.");
                }

                // 繝舌ャ繝・Μ繝ｼ諠・ｱ繧貞叙蠕・
                XINPUT_BATTERY_INFORMATION batteryInfo;
                if (XInputGetBatteryInformation(i, BATTERY_DEVTYPE_GAMEPAD, &batteryInfo) == ERROR_SUCCESS) {
                    DebugLog(std::format("Battery Type: {}", static_cast<int>(batteryInfo.BatteryType)));
                    DebugLog(std::format("Battery Level: {}", static_cast<int>(batteryInfo.BatteryLevel)));
                } else {
                    DebugLog("Failed to get battery information.");
                }
        #endif // QFE_OPTIMIZE_OFF
            }
        } else {
            gamepadStates[i].isConnected = false;
        }
	}
}

/** @brief 繧ｳ繝ｳ繝医Ο繝ｼ繝ｩ繝ｼ縺梧磁邯壹＆繧後※縺・ｋ縺・*/
bool XInputController::GetIsActiveController(uint32_t padId) {
    return gamepadStates[padId].isConnected;
}

/**
 * @brief 繝懊ち繝ｳ縺梧款縺輔ｌ縺ｦ縺・ｋ縺・
 * @param type 繝懊ち繝ｳ縺ｮ遞ｮ鬘・(XINPUT_GAMEPAD_*)
 * @param padId 繝励Ξ繧､繝､繝ｼ繧､繝ｳ繝・ャ繧ｯ繧ｹ
 * @return 謚ｼ縺輔ｌ縺ｦ縺・ｋ縺・
 * TODO: (gamepadStates[padId].state.Gamepad.wButtons == type) 縺ｯ縲∝腰荳繝懊ち繝ｳ縺ｮ縺ｿ蛻､螳壹☆繧句ｴ蜷医↓縺励°讖溯・縺励↑縺・・
 * 隍・焚繝懊ち繝ｳ蜷梧凾謚ｼ縺励・蝣ｴ蜷医・繝薙ャ繝亥柱縺ｫ繧医ｋ蛻､螳・(& type) 縺悟ｿ・ｦ√・
 */
bool XInputController::GetPressButton(WORD type, uint32_t padId) {
    if (padId >= 4) {
        assert(false && padId >= 4);
    }

    if ((gamepadStates[padId].state.Gamepad.wButtons & type) != 0) {
        return true;
    }
    return false;
}

/** @brief 繝懊ち繝ｳ縺梧款縺輔ｌ縺溽椪髢薙° */
bool XInputController::GetTriggerButton(WORD type, uint32_t padId) {
	if (padId >= 4) {
		assert(false && padId >= 4);
	}

	if ((prevGamepadStates[padId].state.Gamepad.wButtons & type) == 0 &&
		(gamepadStates[padId].state.Gamepad.wButtons & type) != 0) {
		return true;
	}
	return false;
}

/** @brief 繝懊ち繝ｳ縺碁屬縺輔ｌ縺溽椪髢薙° */
bool XInputController::GetReleaseButton(WORD type, uint32_t padId) {
    if (padId >= 4) {
        assert(false && padId >= 4);
    }

    if ((prevGamepadStates[padId].state.Gamepad.wButtons & type) != 0 &&
        (gamepadStates[padId].state.Gamepad.wButtons & type) == 0) {
        return true;
    }
    return false;
}

/** @brief 蜿ｳ繧ｹ繝・ぅ繝・け縺ｮ蜈･蜉帙ｒ蜿門ｾ・*/
Vector2 XInputController::GetRightStick(uint32_t padId) {
    if (stickDeadZone_ >= 32767.0f) {
        stickDeadZone_ = 32767.0f;
    }

    if (padId >= 4) {
        assert(false && padId >= 4);
    }
    Vector2 result{};
    result.x = static_cast<float>(gamepadStates[padId].state.Gamepad.sThumbRX);
    result.y = static_cast<float>(gamepadStates[padId].state.Gamepad.sThumbRY);

    // 繝・ャ繝峨だ繝ｼ繝ｳ蜃ｦ逅・
    if (std::fabsf(result.x) <= stickDeadZone_) {
        result.x = 0.0f;
    }
    if (std::fabsf(result.y) <= stickDeadZone_) {
        result.y = 0.0f;
    }

    result.x /= 32767.0f;
    result.y /= 32767.0f;

    return result;
}

/** @brief 蟾ｦ繧ｹ繝・ぅ繝・け縺ｮ蜈･蜉帙ｒ蜿門ｾ・*/
Vector2 XInputController::GetLeftStick(uint32_t padId) {
    if (stickDeadZone_ >= 32767.0f) {
        stickDeadZone_ = 32767.0f;
    }

    if (padId >= 4) {
        assert(false && padId >= 4);
    }
    Vector2 result{};
    result.x = static_cast<float>(gamepadStates[padId].state.Gamepad.sThumbLX);
    result.y = static_cast<float>(gamepadStates[padId].state.Gamepad.sThumbLY);

    // 繝・ャ繝峨だ繝ｼ繝ｳ蜃ｦ逅・
    if (std::fabsf(result.x) <= stickDeadZone_) {
        result.x = 0.0f;
    }
    if (std::fabsf(result.y) <= stickDeadZone_) {
        result.y = 0.0f;
    }

    result.x /= 32767.0f;
    result.y /= 32767.0f;

    return result.Normalize();
}


