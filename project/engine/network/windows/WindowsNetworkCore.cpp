#include "WindowsNetworkCore.h"

#include <winsock2.h>
#include <ws2tcpip.h>
#include <limits>
#include <memory>
#pragma comment(lib, "Ws2_32.lib")

QFE::NETWORK::WindowsNetworkCore::~WindowsNetworkCore() {
	Finalize();
}

QFE::INFO::SupportedOs QFE::NETWORK::WindowsNetworkCore::GetSupportedOs() const
{
	return QFE::INFO::SupportedOs::Windows;
}

bool QFE::NETWORK::WindowsNetworkCore::Initialize()
{
	if (initialized_) return true;
	WSADATA wsaData;
	int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (result != 0) {
		// 初期化失敗
		return false;
	}
	initialized_ = true;
	return true;
}

QFE::NETWORK::ConnectHandle QFE::NETWORK::WindowsNetworkCore::Connect(
	const std::string& host, const std::string& port) {
	if (!initialized_) return {};
	auto connection = std::make_unique<WindowsSinglePeerTransceiver>();
	// 現在の窓口はTCPのIPv4接続を扱う。
	if (!connection->Connect(host, port, NetworkProtocolType::TCP, AddressType::IPv4)) return {};
	return StoreConnection(std::move(connection));
}

QFE::NETWORK::ConnectHandle QFE::NETWORK::WindowsNetworkCore::StoreConnection(
	std::unique_ptr<WindowsSinglePeerTransceiver> connection) {
	for (std::size_t index = 0; index < connections_.size(); ++index) {
		auto& slot = connections_[index];
		// 世代が上限に達した場所は再利用せず、古いハンドルの復活を防ぐ。
		if (!slot.connection && slot.generation != UINT32_MAX) {
			slot.connection = std::move(connection);
			return { static_cast<uint32_t>(index), slot.generation };
		}
	}
	if (connections_.size() >= UINT32_MAX) return {};
	const auto index = static_cast<uint32_t>(connections_.size());
	connections_.push_back({ std::move(connection), 0 });
	return { index, 0 };
}

bool QFE::NETWORK::WindowsNetworkCore::ListenLocal(uint16_t& port) {
	if (!initialized_ || listener_ != INVALID_SOCKET) return false;
	SOCKET listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listener == INVALID_SOCKET) return false;
	sockaddr_in address{};
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	address.sin_port = 0;
	u_long nonBlocking = 1;
	int length = sizeof(address);
	// ポートはOSに割り当ててもらい、他のエディタとの競合を避ける。
	if (bind(listener, reinterpret_cast<const sockaddr*>(&address), sizeof(address)) != 0 ||
		listen(listener, 1) != 0 || ioctlsocket(listener, FIONBIO, &nonBlocking) != 0 ||
		getsockname(listener, reinterpret_cast<sockaddr*>(&address), &length) != 0) {
		closesocket(listener);
		return false;
	}
	listener_ = listener;
	port = ntohs(address.sin_port);
	return true;
}

QFE::NETWORK::AcceptStatus QFE::NETWORK::WindowsNetworkCore::Accept(ConnectHandle& handle) {
	if (!initialized_ || listener_ == INVALID_SOCKET) return AcceptStatus::Error;
	SOCKET socket = accept(listener_, nullptr, nullptr);
	if (socket == INVALID_SOCKET) {
		return WSAGetLastError() == WSAEWOULDBLOCK ? AcceptStatus::Pending : AcceptStatus::Error;
	}
	auto connection = std::make_unique<WindowsSinglePeerTransceiver>();
	if (!connection->AdoptAcceptedSocket(socket)) {
		closesocket(socket);
		return AcceptStatus::Error;
	}
	handle = StoreConnection(std::move(connection));
	return handle.id == UINT32_MAX ? AcceptStatus::Error : AcceptStatus::Accepted;
}

void QFE::NETWORK::WindowsNetworkCore::StopListening() {
	if (listener_ != INVALID_SOCKET) {
		closesocket(listener_);
		listener_ = INVALID_SOCKET;
	}
}

QFE::NETWORK::ITransceiver* QFE::NETWORK::WindowsNetworkCore::FindConnection(ConnectHandle handle) {
	if (!initialized_ || handle.id >= connections_.size()) return nullptr;
	auto& slot = connections_[handle.id];
	return slot.generation == handle.age ? slot.connection.get() : nullptr;
}

bool QFE::NETWORK::WindowsNetworkCore::Disconnect(ConnectHandle handle) {
	if (!FindConnection(handle)) return false;
	auto& slot = connections_[handle.id];
	if (!slot.connection->Disconnect()) return false;
	slot.connection.reset();
	++slot.generation;
	return true;
}

bool QFE::NETWORK::WindowsNetworkCore::Finalize()
{
	if (!initialized_) return true;
	StopListening();
	// Winsockを終了する前に、それを使用する接続を閉じる。
	for (auto& slot : connections_) {
		if (slot.connection) {
			if (!slot.connection->Disconnect()) return false;
			slot.connection.reset();
			++slot.generation;
		}
	}
	int result = WSACleanup();
	if (result != 0) {
		// 終了処理失敗
		return false;
	}
	initialized_ = false;
	return true;
}
