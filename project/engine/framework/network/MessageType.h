#pragma once
#include <cstdint>

namespace QFE::FRAMEWORK::NETWORK {
	/// @brief 通信の種別
	enum class MessageType : uint16_t {
		None = 0,// 未使用
		ConnectRequest = 1,// 接続要求
		ConnectResponse = 2,// 接続応答
		DisconnectRequest = 3,// 切断要求
		DisconnectResponse = 4,// 切断応答

		Message = 5,// メッセージ
	};
}