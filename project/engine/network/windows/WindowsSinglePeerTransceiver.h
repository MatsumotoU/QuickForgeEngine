#pragma once
#include <winsock2.h>
#include <ws2tcpip.h>
#include "network/interface/ITransceiver.h"

namespace QFE::NETWORK {
	/// @brief 一つの相手との通信を扱うWindows用クラス。
	class WindowsSinglePeerTransceiver final : public ITransceiver {
	public:
		WindowsSinglePeerTransceiver() = default;
		~WindowsSinglePeerTransceiver() override;

		// 一つのソケットを複数の所有者が閉じないようにする。
		WindowsSinglePeerTransceiver(const WindowsSinglePeerTransceiver&) = delete;
		WindowsSinglePeerTransceiver& operator=(const WindowsSinglePeerTransceiver&) = delete;
		// 所有者を増やさないようにする
		WindowsSinglePeerTransceiver(WindowsSinglePeerTransceiver&&) = delete;
		WindowsSinglePeerTransceiver& operator=(WindowsSinglePeerTransceiver&&) = delete;

		/// @brief 指定されたホスト、ポート、ネットワークプロトコル、アドレス種別を使用して接続します。
		/// @param host 接続先のホスト名またはアドレス。
		/// @param port 接続先のポート。
		/// @param type 使用するネットワークプロトコルの種類。
		/// @param addressType 使用するアドレスの種類。
		/// @return 接続に成功した場合は true、失敗した場合は false。
		bool Connect(
			const std::string& host, const std::string& port,
			NetworkProtocolType type, AddressType addressType) override;

		/// @brief 接続を切断します。
		/// @return 切断に成功した場合は true、それ以外の場合は false。
		bool Disconnect() override;

		/// @brief 接続が有効かどうかを確認します。
		/// @return 接続が有効な場合は true、それ以外の場合は false。
		bool IsConnected() const override;

		/// @brief データをブロッキング方式で送信します。
		/// @param data 送信するデータへのポインターです。
		/// @param size 送信するデータのサイズ（バイト単位）です。
		/// @return 送信に成功した場合は true、失敗した場合は false を返します。
		bool SendBlocking(const uint8_t* data, std::size_t size) override;

		/// @brief データをブロッキング方式で受信します。
		/// @param data 受信したデータを書き込むバッファへのポインター。
		/// @param size 受信するデータのサイズ（バイト単位）。
		/// @return データの受信に成功した場合は true、失敗した場合は false。
		bool ReceiveBlocking(uint8_t* data, std::size_t size) override;
		ReceiveStatus ReceiveAvailable(std::vector<uint8_t>& data) override;

		// acceptで得たソケットの所有権を受け取る。Windows内部だけで使う。
		bool AdoptAcceptedSocket(SOCKET socket);

	private:
		SOCKET socket_ = INVALID_SOCKET;
		NetworkProtocolType type_ = NetworkProtocolType::INVALID;
		AddressType addressType_ = AddressType::INVALID;
	};
}
