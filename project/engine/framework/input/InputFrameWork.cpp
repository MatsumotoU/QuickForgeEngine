#include "InputFrameWork.h"
#include "input/InputInterface.h"

std::unique_ptr<QFE::INPUT::InputInterface> QFE::FRAMEWORK::CreateInputInterface(const HWND& hwnd, const HINSTANCE& hInstance) {
	// 引数の確認
	if(hwnd == nullptr || hInstance == nullptr) {
		QFE_LOG("HWND or HINSTANCE is null");
	}

	// InputInterfaceの生成&初期化
	std::unique_ptr<QFE::INPUT::InputInterface> inputInterface = std::make_unique<QFE::INPUT::InputInterface>();
	inputInterface->Initialize(hwnd, hInstance);
	return inputInterface;
}
