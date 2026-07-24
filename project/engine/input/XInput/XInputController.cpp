#include "XInputController.h"
#pragma comment(lib, "Xinput.lib")

#include "EngineDefines.h"

#include <assert.h>
#include <cmath>

namespace QFE::INPUT {

	/** @brief コンストラクタ */
	XInputController::XInputController() {
		stickDeadZone_ = 7849.0f;
	}

	/** @brief 更新 */
	void XInputController::Update() {
		prevGamepadStates = gamepadStates;

		for (DWORD i = 0; i < XUSER_MAX_COUNT; i++) {
			DWORD dwResult = XInputGetState(i, &gamepadStates[i].state);

			if (dwResult == ERROR_SUCCESS) {
				if (!gamepadStates[i].isConnected) {
					gamepadStates[i].isConnected = true;
#ifdef QFE_OPTIMIZE_OFF
					QFE_LOG(std::format("Connected Controller! paletNumber: {}", gamepadStates[i].state.dwPacketNumber));

					// 機能情報を取得
					XINPUT_CAPABILITIES cap;
					if (XInputGetCapabilities(i, 0, &cap) == ERROR_SUCCESS) {
						QFE_LOG(std::format("Type: {}", static_cast<int>(cap.Type)));
						QFE_LOG(std::format("SubType: {}", static_cast<int>(cap.SubType)));
						QFE_LOG(std::format("Flags: 0x{:X}", static_cast<unsigned int>(cap.Flags)));
					} else {
						QFE_LOG("Failed to get capabilities.");
					}

					// バッテリー情報を取得
					XINPUT_BATTERY_INFORMATION batteryInfo;
					if (XInputGetBatteryInformation(i, BATTERY_DEVTYPE_GAMEPAD, &batteryInfo) == ERROR_SUCCESS) {
						QFE_LOG(std::format("Battery Type: {}", static_cast<int>(batteryInfo.BatteryType)));
						QFE_LOG(std::format("Battery Level: {}", static_cast<int>(batteryInfo.BatteryLevel)));
					} else {
						QFE_LOG("Failed to get battery information.");
					}
#endif // QFE::INPUT_OPTIMIZE_OFF
				}
			} else {
				gamepadStates[i].isConnected = false;
			}
		}
	}

	/** @brief コントローラーが接続されているか */
	bool XInputController::GetIsActiveController(uint32_t padId) const {
		if (padId >= XUSER_MAX_COUNT) {
			return false;
		}
		return gamepadStates[padId].isConnected;
	}

	/**
	 * @brief ボタンが押されているか
	 * @param type ボタンの種類 (XINPUT_GAMEPAD_*)
	 * @param padId プレイヤーインデックス
	 * @return 押されているか
	 */
	bool XInputController::GetPressButton(WORD type, uint32_t padId) {
		if (padId >= XUSER_MAX_COUNT) {
			return false;
		}

		if ((gamepadStates[padId].state.Gamepad.wButtons & type) != 0) {
			return true;
		}
		return false;
	}

	/** @brief ボタンが押された瞬間か */
	bool XInputController::GetTriggerButton(WORD type, uint32_t padId) {
		if (padId >= XUSER_MAX_COUNT) {
			return false;
		}

		if ((prevGamepadStates[padId].state.Gamepad.wButtons & type) == 0 &&
			(gamepadStates[padId].state.Gamepad.wButtons & type) != 0) {
			return true;
		}
		return false;
	}

	/** @brief ボタンが離された瞬間か */
	bool XInputController::GetReleaseButton(WORD type, uint32_t padId) {
		if (padId >= XUSER_MAX_COUNT) {
			return false;
		}

		if ((prevGamepadStates[padId].state.Gamepad.wButtons & type) != 0 &&
			(gamepadStates[padId].state.Gamepad.wButtons & type) == 0) {
			return true;
		}
		return false;
	}

	/** @brief 右スティックの入力を取得 */
	MATH::Vector2 XInputController::GetRightStick(uint32_t padId) {
		if (stickDeadZone_ >= 32767.0f) {
			stickDeadZone_ = 32767.0f;
		}

		if (padId >= XUSER_MAX_COUNT) {
			return {};
		}
		MATH::Vector2 result{};
		result.x = static_cast<float>(gamepadStates[padId].state.Gamepad.sThumbRX);
		result.y = static_cast<float>(gamepadStates[padId].state.Gamepad.sThumbRY);

		// デッドゾーン処理
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

	/** @brief 左スティックの入力を取得 */
	MATH::Vector2 XInputController::GetLeftStick(uint32_t padId) {
		if (stickDeadZone_ >= 32767.0f) {
			stickDeadZone_ = 32767.0f;
		}

		if (padId >= XUSER_MAX_COUNT) {
			return {};
		}
		MATH::Vector2 result{};
		result.x = static_cast<float>(gamepadStates[padId].state.Gamepad.sThumbLX);
		result.y = static_cast<float>(gamepadStates[padId].state.Gamepad.sThumbLY);

		// デッドゾーン処理
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

}
