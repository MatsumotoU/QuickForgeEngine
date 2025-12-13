#include "engine/include/window/WindowGenerater/WindowGenerater.h"

void WindowGenerater::CreateGameWindow(WNDCLASS& wc, HWND& hwnd, WindowConfigData& config, WNDPROC& proc, WindowEventsManager* eventManager) {
	// 繧ｦ繧｣繝ｳ繝峨え繝励Ο繝ｼ繧ｸ繝｣繝ｼ
	wc.lpfnWndProc = proc;
	// 繧ｦ繧｣繝ｳ繝峨え繧ｯ繝ｩ繧ｹ蜷・
	wc.lpszClassName = L"CG2WindowClass";
	// 繧､繝ｳ繧ｹ繧ｿ繝ｳ繧ｹ繝上Φ繝峨Ν
	wc.hInstance = GetModuleHandle(nullptr);
	// 繧ｫ繝ｼ繧ｽ繝ｫ
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

	// 繧ｦ繧｣繝ｳ繝峨え繧ｯ繝ｩ繧ｹ繧堤匳骭ｲ縺吶ｋ
	RegisterClass(&wc);

	// 繧ｦ繧｣繝ｳ繝峨え繧ｵ繧､繧ｺ繧定｡ｨ縺呎ｧ矩菴薙↓繧ｯ繝ｩ繧､繧｢繝ｳ繝磯伜沺繧貞・繧後ｋ
	RECT wrc = { 0,0,config.clientWidth,config.clientHeight };

	// 繧ｯ繝ｩ繧､繧｢繝ｳ繝磯伜沺繧貞・縺ｫ螳滄圀縺ｮ繧ｵ繧､繧ｺ縺ｫwrc繧呈峩譁ｰ縺励※繧ゅｉ縺・
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

	// 繝ｪ繧ｵ繧､繧ｺ遖∵ｭ｢
	DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
	
	// 繧ｦ繧｣繝ｳ繝峨え縺ｮ逕滓・
	hwnd = CreateWindow(
		wc.lpszClassName,
		config.windowName.c_str(),
		style,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		wrc.right - wrc.left,
		wrc.bottom - wrc.top,
		nullptr,
		nullptr,
		wc.hInstance,
		eventManager
	);

	ShowWindow(hwnd, SW_SHOW);

	// 繧ｦ繧｣繝ｳ繝峨え縺ｮ讖溯・隗｣謾ｾ
	if (config.isCanDropFiles) {
		// 繧ｦ繧｣繝ｳ繝峨え縺ｫ蟇ｾ縺励※縲√ラ繝ｩ繝・げ・・ラ繝ｭ繝・・繧呈怏蜉ｹ縺ｫ縺吶ｋ
		DragAcceptFiles(hwnd, TRUE);
	}

}
