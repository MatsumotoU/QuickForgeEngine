#include "WindowsSinglePeerTransceiver.h"
#include <algorithm>
#include <limits>

using namespace QFE::NETWORK;

WindowsSinglePeerTransceiver::~WindowsSinglePeerTransceiver() {
	Disconnect();
}

bool WindowsSinglePeerTransceiver::AdoptAcceptedSocket(SOCKET socket) {
	if (IsConnected() || socket == INVALID_SOCKET) return false;
	// 待ち受け側は非ブロッキングのため、受理した通信は明示的に待機方式へ戻す。
	u_long nonBlocking = 0;
	if (ioctlsocket(socket, FIONBIO, &nonBlocking) != 0) return false;
	socket_ = socket;
	type_ = NetworkProtocolType::TCP;
	addressType_ = AddressType::IPv4;
	return true;
}

bool WindowsSinglePeerTransceiver::Connect(const std::string& host, const std::string& port,
	NetworkProtocolType type, AddressType addressType) {
	// 既存の接続を暗黙に失わないよう、明示的な切断を要求する。
	if (IsConnected() || type != NetworkProtocolType::TCP) return false;
	addrinfo hints{};
	if (addressType == AddressType::IPv4) hints.ai_family = AF_INET;
	else if (addressType == AddressType::IPv6) hints.ai_family = AF_INET6;
	else return false;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	addrinfo* addresses = nullptr;
	if (getaddrinfo(host.c_str(), port.c_str(), &hints, &addresses) != 0) return false;
	// 名前解決が複数候補を返す場合は、接続できる候補を順に探す。
	for (const addrinfo* address = addresses; address != nullptr; address = address->ai_next) {
		SOCKET candidate = socket(address->ai_family, address->ai_socktype, address->ai_protocol);
		if (candidate == INVALID_SOCKET) continue;
		if (connect(candidate, address->ai_addr, static_cast<int>(address->ai_addrlen)) == 0) {
			socket_ = candidate;
			break;
		}
		closesocket(candidate);
	}
	freeaddrinfo(addresses);
	if (!IsConnected()) return false;
	type_ = type;
	addressType_ = addressType;
	return true;
}

bool WindowsSinglePeerTransceiver::Disconnect() {
	if (!IsConnected()) return true;
	const int result = closesocket(socket_);
	if (result != 0) return false;
	socket_ = INVALID_SOCKET;
	type_ = NetworkProtocolType::INVALID;
	addressType_ = AddressType::INVALID;
	return true;
}

bool WindowsSinglePeerTransceiver::IsConnected() const {
	return socket_ != INVALID_SOCKET;
}

bool WindowsSinglePeerTransceiver::SendBlocking(const uint8_t* data, std::size_t size) {
	if (!IsConnected() || (size != 0 && data == nullptr)) return false;
	while (size > 0) {
		// Winsockの長さはintなので、長いデータは複数回に分ける。
		const int chunk = static_cast<int>((std::min)(size, static_cast<std::size_t>((std::numeric_limits<int>::max)())));
		const int sent = send(socket_, reinterpret_cast<const char*>(data), chunk, 0);
		if (sent <= 0) { Disconnect(); return false; }
		data += sent;
		size -= static_cast<std::size_t>(sent);
	}
	return true;
}

bool WindowsSinglePeerTransceiver::ReceiveBlocking(uint8_t* data, std::size_t size) {
	if (!IsConnected() || (size != 0 && data == nullptr)) return false;
	while (size > 0) {
		const int chunk = static_cast<int>((std::min)(size, static_cast<std::size_t>((std::numeric_limits<int>::max)())));
		const int received = recv(socket_, reinterpret_cast<char*>(data), chunk, 0);
		// 途中で失敗した通信は境界が不明になるため、接続を閉じる。
		if (received <= 0) { Disconnect(); return false; }
		data += received;
		size -= static_cast<std::size_t>(received);
	}
	return true;
}

ReceiveStatus WindowsSinglePeerTransceiver::ReceiveAvailable(std::vector<uint8_t>& data) {
	if (!IsConnected()) return ReceiveStatus::Closed;
	fd_set readSet;
	FD_ZERO(&readSet);
	FD_SET(socket_, &readSet);
	timeval timeout{};
	const int ready = select(0, &readSet, nullptr, nullptr, &timeout);
	if (ready == 0) return ReceiveStatus::Pending;
	if (ready == SOCKET_ERROR) { Disconnect(); return ReceiveStatus::Error; }
	char buffer[8192];
	const int count = recv(socket_, buffer, sizeof(buffer), 0);
	if (count == 0) { Disconnect(); return ReceiveStatus::Closed; }
	if (count == SOCKET_ERROR) { Disconnect(); return ReceiveStatus::Error; }
	// 一度に取得できる範囲だけを返し、メッセージの組み立てはFrameworkに任せる。
	data.assign(buffer, buffer + count);
	return ReceiveStatus::Data;
}
