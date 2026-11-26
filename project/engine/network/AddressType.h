#pragma once
namespace QFE::NETWORK {
	/// @brief 通信アドレスの種類を表す列挙型
	enum class AddressType {
		INVALID,
		IPv4,
		IPv6
	};
}