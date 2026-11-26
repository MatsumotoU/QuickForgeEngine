#pragma once
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>
#include "network/NetworkProtocolType.h"
#include "network/AddressType.h"

namespace QFE::NETWORK {
	enum class ReceiveStatus { Data, Pending, Closed, Error };
	/// @brief 一つの相手との接続とバイト列の送受信を扱う。
	class ITransceiver {
	public:
		virtual ~ITransceiver() = default;
		virtual bool Connect(const std::string& host, const std::string& port,
			NetworkProtocolType type, AddressType addressType) = 0;
		virtual bool Disconnect() = 0;
		/// @brief ローカルで接続を保持しているかを返す。相手の生存確認ではない。
		virtual bool IsConnected() const = 0;
		/// @brief 指定バイト数を送るまで呼び出し元のスレッドを待たせる。
		/// @details size自体は送信しない。サイズや種別の通知は上位層の責任。
		virtual bool SendBlocking(const uint8_t* data, std::size_t size) = 0;
		/// @brief 指定バイト数を受け取るまで呼び出し元のスレッドを待たせる。
		/// @details 失敗時は途中まで書き込まれる場合がある。
		virtual bool ReceiveBlocking(uint8_t* data, std::size_t size) = 0;
		// 今届いているバイトだけを取得し、届いていない場合は待たずに戻る。
		virtual ReceiveStatus ReceiveAvailable(std::vector<uint8_t>& data) = 0;
	};
}
