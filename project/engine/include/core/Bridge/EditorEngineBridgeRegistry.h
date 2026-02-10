#pragma once
namespace QFE {
	class WindowsEngineCore;

	/// @brief エディタとエンジン間の橋渡し関数を登録・解除するクラス
	class EditorEngineBridgeRegistry final{
	public:
		static void RegisterFunctions(WindowsEngineCore* engineCore);
		static void ClearFunctions();
	};
}