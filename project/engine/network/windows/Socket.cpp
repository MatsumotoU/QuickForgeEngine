#include "Soket.h"
#include <unordered_map>
using namespace QFE::NETWORK;

namespace {
	// AddressTypeとAF_INET/AF_INET6のマッピング
	const std::unordered_map<AddressType, int> kAddressTypeMap = {
		{ AddressType::IPv4, AF_INET },
		{ AddressType::IPv6, AF_INET6 }
	};
	// NetworkProtocolTypeとSOCK_STREAM/SOCK_DGRAMのマッピング
	const std::unordered_map<NetworkProtocolType, int> kProtocolTypeMap = {
		{ NetworkProtocolType::TCP, SOCK_STREAM }
	};
}

bool QFE::NETWORK::Socket::Create(
	const std::string& host, const std::string& port,
	NetworkProtocolType type, AddressType addressType){

	// ソケットの種類とアドレスファミリを設定
	addrinfo hints{};
	hints.ai_family = kAddressTypeMap.at(addressType);
	hints.ai_socktype = kProtocolTypeMap.at(type);
	addrinfo* result = nullptr;
	// ホスト名とポート番号を解決
	if (getaddrinfo(host.c_str(), port.c_str(), &hints, &result) != 0) {
		return false;
	}
	
	// ソケットを作成
	socket_ = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	freeaddrinfo(result);
	if (socket_ == INVALID_SOCKET) {
		return false;
	}

	// ソケットの種類とアドレスファミリを保存
	type_ = type;
	addressType_ = addressType;

	return true;
}

void Socket::Close() {
	// ソケットが有効な場合は閉じる
	if(socket_ != INVALID_SOCKET) {
		closesocket(socket_);
		socket_ = INVALID_SOCKET;
		type_ = NetworkProtocolType::INVALID;
		addressType_ = AddressType::INVALID;
	}
}

bool Socket::IsValid() const {
	return socket_ != INVALID_SOCKET;
}
