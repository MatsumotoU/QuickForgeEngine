#pragma once
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include "NetworkProtocolType.h"
#include "AddressType.h"

namespace QFE::NETWORK {
	/// @brief winsock2のソケット機能のラッパー
	class Socket {
	public:
		/// @brief ソケットを作成します.
		bool Create(
			const std::string& host, const std::string& port,
			NetworkProtocolType type, AddressType addressType);
		/// @brief ソケットを閉じます.
		void Close();

		/// @brief ソケットが有効かどうかを返します.
		bool IsValid() const;

	private:
		SOCKET socket_ = INVALID_SOCKET;
		NetworkProtocolType type_ = NetworkProtocolType::INVALID;
		AddressType addressType_ = AddressType::INVALID;
		
	};
}