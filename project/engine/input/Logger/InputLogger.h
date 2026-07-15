#pragma once
#include "InputLogData.h"
#include <vector>
#include <map>

namespace QFE::INPUT {
	class InputLogger final {
	public:
		InputLogger();
		~InputLogger() = default;

		// 新しいフレームの開始処理
		void StartNewFrame();
		// フレーム終了処理
		void EndFrame();

		// 録画を開始する
		void StartRecording();
		// 録画を停止する
		void StopRecording();

		// キー入力を記録する
		void RecordKeyPress(uint32_t keyCode);

		// 録画された入力ログデータを取得する
		const std::vector<InputLogData>& GetInputLogDataList() const;

		const bool IsRecording() const { return isRecording_; }

	private:
		bool isRecording_;

		// 今回のフレームで押されたキーコード一覧
		std::vector<uint32_t> currentFramePressedKeyCodes_;
		// 前のフレームで押されたキーコード一覧
		std::vector<uint32_t> previousFramePressedKeyCodes_;

		uint32_t frameCounter_;
		// 押し続けているキーコード一覧
		std::map<uint32_t, InputLogData> holdingKeyDataList_;
		// 入力ログデータ一覧
		std::vector<InputLogData> inputLogDataList_;
	};
}  // namespace QFE::INPUT