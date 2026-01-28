#include "Engine/include/input/Logger/InputLogger.h"
using namespace QFE;
InputLogger::InputLogger() :
	isRecording_(false),
	frameCounter_(0) {}

void InputLogger::StartNewFrame()
{
	if (!isRecording_) { return; }

	frameCounter_++;

	previousFramePressedKeyCodes_ = currentFramePressedKeyCodes_;
	currentFramePressedKeyCodes_.clear();
}

void InputLogger::EndFrame()
{
	if (!isRecording_) { return; }

	// 今回のフレームで押されたキーコードを処理
	for (const auto& keyCode : currentFramePressedKeyCodes_) {
		// 前のフレームで押されていなかった場合、新規入力として記録
		if (std::find(previousFramePressedKeyCodes_.begin(), previousFramePressedKeyCodes_.end(), keyCode) == previousFramePressedKeyCodes_.end()) {
			InputLogData logData;
			logData.pressedKeyCode_ = keyCode;
			logData.startFrame_ = frameCounter_;
			holdingKeyDataList_.emplace(keyCode,logData);
		}
	}

	// 前回と比べて今回押されていなかったキーを処理
	for (const auto& keyCode : previousFramePressedKeyCodes_) {
		// 今回のフレームで押されていなかった場合、離された入力として記録
		if (std::find(currentFramePressedKeyCodes_.begin(), currentFramePressedKeyCodes_.end(), keyCode) == currentFramePressedKeyCodes_.end()) {
			auto it = holdingKeyDataList_.find(keyCode);
			if (it != holdingKeyDataList_.end()) {
				it->second.endFrame_ = frameCounter_;
				inputLogDataList_.emplace_back(it->second);
				holdingKeyDataList_.erase(it);
			}
		}
	}
}

void InputLogger::StartRecording()
{
	isRecording_ = true;
	inputLogDataList_.clear();
	holdingKeyDataList_.clear();
	frameCounter_ = 0;
}

void InputLogger::StopRecording()
{
	isRecording_ = false;
	// 押し続けているキーの終了フレームを設定してログに追加
	for (auto& pair : holdingKeyDataList_) {
		pair.second.endFrame_ = frameCounter_;
		inputLogDataList_.emplace_back(pair.second);
	}
	holdingKeyDataList_.clear();
}

void InputLogger::RecordKeyPress(uint32_t keyCode)
{
	if (!isRecording_) { return; }

	// すでに記録されている場合は追加しない
	if (std::find(currentFramePressedKeyCodes_.begin(), currentFramePressedKeyCodes_.end(), keyCode) == currentFramePressedKeyCodes_.end()) {
		currentFramePressedKeyCodes_.emplace_back(keyCode);
	}
}

const std::vector<InputLogData>& InputLogger::GetInputLogDataList() const
{
	return inputLogDataList_;
}
