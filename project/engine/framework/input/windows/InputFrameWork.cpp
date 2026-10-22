#include "framework/input/InputFrameWork.h"
#include "input/WindowsInput.h"

std::unique_ptr<QFE::INPUT::InputInterface> QFE::FRAMEWORK::CreateInputInterface(HWND hwnd, HINSTANCE hInstance) {
	if(hwnd == nullptr || hInstance == nullptr) {
		QFE_LOG("HWND or HINSTANCE is null");
	}

	auto inputInterface = std::make_unique<QFE::INPUT::WindowsInput>();
	inputInterface->Initialize(hwnd, hInstance);
	return inputInterface;
}
