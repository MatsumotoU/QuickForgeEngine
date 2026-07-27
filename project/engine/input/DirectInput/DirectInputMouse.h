#pragma once
#include <stdint.h>

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

#include "math/MathInclude.h"

namespace QFE::INPUT {
	class DirectInputMouse {
	public:
		DirectInputMouse();
		~DirectInputMouse();

	public:// コアの関数
		void Initialize(const HWND& hwnd, IDirectInput8* directInput);
		void Update();

	public:// マウス管理関数
		/// <summary>
		/// 任意のキーを押し続けているか
		/// </summary>
		/// <param name="DIK">左クリック[0],右クリック[1],中クリック[2]</param>
		/// <returns></returns>
		bool GetPress(int8_t DIK);
		/// <summary>
		/// 任意のキーを押したか
		/// </summary>
		/// <param name="DIK">DIK_Key</param>
		/// <returns></returns>
		bool GetTrigger(int8_t DIK);
		/// <summary>
		/// 任意のキーを離したか
		/// </summary>
		/// <param name="DIK">DIK_Key</param>
		/// <returns></returns>
		bool GetRelease(int8_t DIK);

	public:// マウス操作変数
		DIMOUSESTATE mouseState_;
		DIMOUSESTATE preMouseState_;
		MATH::Vector2 mouseScreenPos_;
		MATH::Vector2 preMouseScreenPos_;
		MATH::Vector2 mouseMoveDir_;
		MATH::Vector2 deltaMouse_;
		float wheelDir_;

	private:
		IDirectInputDevice8* CreateMouse();

	private:
		IDirectInputDevice8* mouse_;
		IDirectInput8* directInput_;
		HWND hwnd_;
		POINT mousePos_;
	};
}  // namespace QFE::INPUT