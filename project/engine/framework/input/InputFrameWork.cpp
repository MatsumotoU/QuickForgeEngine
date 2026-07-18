#include "InputFrameWork.h"
#include "input/InputInterface.h"
#include "input/WindowsInput.h"

std::unique_ptr<QFE::INPUT::InputInterface> QFE::FRAMEWORK::CreateInputInterface(HWND hwnd, HINSTANCE hInstance) {
	// 引数の確認
	if(hwnd == nullptr || hInstance == nullptr) {
		QFE_LOG("HWND or HINSTANCE is null");
	}

	// InputInterfaceの生成&初期化
	std::unique_ptr<QFE::INPUT::WindowsInput> inputInterface = std::make_unique<QFE::INPUT::WindowsInput>();
	inputInterface->Initialize(hwnd, hInstance);
	return inputInterface;
}

bool QFE::FRAMEWORK::UpdateInputInterface(QFE::INPUT::InputInterface* inputInterface) {
	inputInterface->Update();
	return true;
}

bool QFE::FRAMEWORK::EndFrameInputInterface(QFE::INPUT::InputInterface* inputInterface) {
	inputInterface->EndFrame();
	return true;
}
