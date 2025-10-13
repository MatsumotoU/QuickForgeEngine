#include "XInputController.h"
#pragma comment(lib, "Xinput.lib")

#ifdef _DEBUG
#include "Utility/MyDebugLog.h"
#endif // _DEBUG

#include <assert.h>
#include <cmath>

XInputController::XInputController() {
    stickDeadZone_ = 7849.0f;
}

void XInputController::Update() {
	for (DWORD i = 0; i < XUSER_MAX_COUNT; i++) {
		DWORD dwResult = XInputGetState(i, &gamepadStates[i].state);

        if (dwResult == ERROR_SUCCESS) {
            if (!gamepadStates[i].isConnected) {
                gamepadStates[i].isConnected = true;
        #ifdef _DEBUG
                DebugLog(std::format("Connected Controller! paletNumber: {}", gamepadStates[i].state.dwPacketNumber));

                // 機能情報を取得
                XINPUT_CAPABILITIES cap;
                if (XInputGetCapabilities(i, 0, &cap) == ERROR_SUCCESS) {
                    DebugLog(std::format("Type: {}", static_cast<int>(cap.Type)));
                    DebugLog(std::format("SubType: {}", static_cast<int>(cap.SubType)));
                    DebugLog(std::format("Flags: 0x{:X}", static_cast<unsigned int>(cap.Flags)));
                } else {
                    DebugLog("Failed to get capabilities.");
                }

                // バッテリー情報を取得
                XINPUT_BATTERY_INFORMATION batteryInfo;
                if (XInputGetBatteryInformation(i, BATTERY_DEVTYPE_GAMEPAD, &batteryInfo) == ERROR_SUCCESS) {
                    DebugLog(std::format("Battery Type: {}", static_cast<int>(batteryInfo.BatteryType)));
                    DebugLog(std::format("Battery Level: {}", static_cast<int>(batteryInfo.BatteryLevel)));
                } else {
                    DebugLog("Failed to get battery information.");
                }
        #endif // _DEBUG
            }
        } else {
            gamepadStates[i].isConnected = false;
        }
	}
}

bool XInputController::GetIsActiveController(uint32_t padId) {
    return gamepadStates[padId].isConnected;
}

bool XInputController::GetPressButton(WORD type, uint32_t padId) {
    if (padId >= 4) {
        assert(false && padId >= 4);
    }

    if (gamepadStates[padId].state.Gamepad.wButtons == type) {
        return true;
    }
    return false;
}

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

    // 足切り
    if (std::fabsf(result.x) <= stickDeadZone_) {
        result.x = 0.0f;
    }
    if (std::fabsf(result.y) <= stickDeadZone_) {
        result.y = 0.0f;
    }

    result.x /= stickDeadZone_;
    result.y /= stickDeadZone_;

    return result;

    return result;
}

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

    // 足切り
    if (std::fabsf(result.x) <= stickDeadZone_) {
        result.x = 0.0f;
    }
    if (std::fabsf(result.y) <= stickDeadZone_) {
        result.y = 0.0f;
    }

    result.x /= stickDeadZone_;
    result.y /= stickDeadZone_;

    return result.Normalize();
}
