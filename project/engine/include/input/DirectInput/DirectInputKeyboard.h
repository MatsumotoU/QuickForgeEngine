#pragma once
#include <stdint.h>
#include "engine/include/core/Memory/SafeVector.h"

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
namespace QFE {
	class DirectInputKeyboard {
	public:
		DirectInputKeyboard();
		~DirectInputKeyboard();

	public:// コアの関数
		void Initialize(const HWND& hwnd, IDirectInput8* directInput);
		void Update();

	public:// キー管理関数
		/// <summary>
		/// 任意のキーを押し続けているか
		/// </summary>
		/// <param name="DIK">DIK_Key</param>
		/// <returns></returns>
		bool GetPress(uint32_t DIK);
		/// <summary>
		/// 任意のキーを押したか
		/// </summary>
		/// <param name="DIK">DIK_Key</param>
		/// <returns></returns>
		bool GetTrigger(uint32_t DIK);
		/// <summary>
		/// 任意のキーを離したか
		/// </summary>
		/// <param name="DIK">DIK_Key</param>
		/// <returns></returns>
		bool GetRelease(uint32_t DIK);

		/// <summary>
		/// 押されているキーのリストを取得
		/// </summary>
		const SafeVector<uint32_t>& GetPressedKeys();

	public:// キーボード操作変数
		BYTE key_[256];
		BYTE preKey_[256];
		SafeVector<uint32_t> pressedKeys_;

	private:
		IDirectInputDevice8* CreateKeyboard();

	private:
		IDirectInputDevice8* keyboard_;
		IDirectInput8* directInput_;
		HWND hwnd_;
	};
}  // namespace QFE
