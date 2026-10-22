#include "WindowsNetworkCore.h"

#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

QFE::INFO::SupportedOs QFE::NETWORK::WindowsNetworkCore::GetSupportedOs() const
{
	return QFE::INFO::SupportedOs::Windows;
}

bool QFE::NETWORK::WindowsNetworkCore::Initialize()
{
	WSADATA wsaData;
	int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (result != 0) {
		// 初期化失敗
		return false;
	}
	return true;
}
