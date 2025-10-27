#include "InputInterface.h"

void InputInterface::Initialize(const HWND& hwnd, const HINSTANCE& hInstance) {
	keyConfig_.Initialize();
	keyConfig_.LoadKeyConfig();
	directInputManager_.Initialize(hwnd, hInstance);
}

void InputInterface::Finalize() {
	keyConfig_.SaveKeyConfig();
	keyConfig_.Finalize();
	directInputManager_.Finalize();
}

void InputInterface::Update() {
	directInputManager_.Update();
	xInputController_.Update();
}

uint32_t InputInterface::GetKeyCodeTrigger() {
	for (uint32_t keyCode = 0; keyCode < 256; keyCode++) {
		if (directInputManager_.keyboard_.GetPress(keyCode)) {
			return keyCode;
		}
	}
	return 0;
}

bool InputInterface::IsAnyKeyPressed() {
	for (uint32_t keyCode = 0; keyCode < 256; keyCode++) {
		if (directInputManager_.keyboard_.GetPress(keyCode)) {
			return true;
		}
	}
	return false;
}

bool InputInterface::GetKeyPress(const std::string& actionName) {
	bool isPressed = false;
	for (const auto& keyCode : keyConfig_.GetKeys(actionName)) {
		isPressed |= directInputManager_.keyboard_.GetPress(keyCode);
	}
	return isPressed;
}

bool InputInterface::GetKeyTrigger(const std::string& actionName) {
	bool isTriggered = false;
	for (const auto& keyCode : keyConfig_.GetKeys(actionName)) {
		isTriggered |= directInputManager_.keyboard_.GetTrigger(keyCode);
	}
	return isTriggered;
}

bool InputInterface::GetKeyRelease(const std::string& actionName) {
	bool isReleased = false;
	for (const auto& keyCode : keyConfig_.GetKeys(actionName)) {
		isReleased |= directInputManager_.keyboard_.GetRelease(keyCode);
	}
	return isReleased;
}

Vector2 InputInterface::GetKeyMoveDir() {
	return directInputManager_.GetKeyMoveDir();
}

bool InputInterface::GetMousePress(int8_t button) {
	return directInputManager_.mouse_.GetPress(button);
}

bool InputInterface::GetMouseTrigger(int8_t button) {
	return directInputManager_.mouse_.GetTrigger(button);
}

bool InputInterface::GetMouseRelease(int8_t button) {
	return directInputManager_.mouse_.GetRelease(button);
}

Vector2 InputInterface::GetMouseMove() {
	return directInputManager_.mouse_.mouseMoveDir_;
}

Vector2 InputInterface::GetMouseScreenPos() { 
	return directInputManager_.mouse_.mouseScreenPos_; 
}

float InputInterface::GetMouseWheelDir() {
	return directInputManager_.mouse_.wheelDir_;
}

bool InputInterface::GetGamePadPress(uint16_t button) {
	return xInputController_.GetPressButton(static_cast<WORD>(button), 0);
}

bool InputInterface::GetGamePadTrigger(uint16_t button) {
	return xInputController_.GetTriggerButton(static_cast<WORD>(button), 0);
}

bool InputInterface::GetGamePadRelease(uint16_t button) {
	return xInputController_.GetReleaseButton(static_cast<WORD>(button),0);
}

Vector2 InputInterface::GetGamePadLeftStickDir() {
	return xInputController_.GetLeftStick(0);
}

Vector2 InputInterface::GetGamePadRightStickDir() {
	return xInputController_.GetRightStick(0);
}

void InputInterface::AddKeyConfig(const std::string& actionName, uint32_t keyCorde) {
	keyConfig_.AddKey(actionName, keyCorde);
}

void InputInterface::ClearKeyConfig(const std::string& actionName) {
	keyConfig_.RemoveKey(actionName);
}

const std::vector<uint32_t>& InputInterface::GetKeyConfig(const std::string& actionName) {
	return keyConfig_.GetKeys(actionName);
}
