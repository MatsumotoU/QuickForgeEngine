#include "framework/network/NetworkCoreFrame.h"
#include "network/interface/INetworkCore.h"
#include "network/interface/ITransceiver.h"
#include "network/windows/WindowsNetworkCore.h"
#include <array>
#include <cstdint>
#include <limits>
#include <utility>

namespace QFE::FRAMEWORK::NETWORK {
	std::unique_ptr<QFE::NETWORK::INetworkCore> CreateNetworkCore() {
		auto core = std::make_unique<QFE::NETWORK::WindowsNetworkCore>();
		if (!core->Initialize()) return nullptr;
		return core;
	}

	bool ShutdownNetworkCore(QFE::NETWORK::INetworkCore* core) {
		auto* windowsCore = dynamic_cast<QFE::NETWORK::WindowsNetworkCore*>(core);
		return windowsCore && windowsCore->Finalize();
	}

	QFE::NETWORK::ConnectHandle Connect(QFE::NETWORK::INetworkCore* core,
		const std::string& host, const std::string& port) {
		return core ? core->Connect(host, port) : QFE::NETWORK::ConnectHandle{};
	}

	bool Disconnect(QFE::NETWORK::INetworkCore* core, QFE::NETWORK::ConnectHandle handle) {
		return core && core->Disconnect(handle);
	}

	bool ListenLocal(QFE::NETWORK::INetworkCore* core, uint16_t& port) {
		return core && core->ListenLocal(port);
	}
	AcceptStatus Accept(QFE::NETWORK::INetworkCore* core, QFE::NETWORK::ConnectHandle& handle) {
		return core ? core->Accept(handle) : AcceptStatus::Error;
	}
	void StopListening(QFE::NETWORK::INetworkCore* core) {
		if (core) core->StopListening();
	}
	PollStatus PollMessage(QFE::NETWORK::INetworkCore* core, QFE::NETWORK::ConnectHandle handle,
		MessageReader& reader, MessageType& type, std::string& body) {
		if (!core) return PollStatus::Error;
		auto* connection = core->FindConnection(handle);
		if (!connection) return PollStatus::Error;
		// 既に完成したメッセージは、新しい受信より先に取り出す。
		auto extract = [&]() {
			if (reader.pending.size() < 4) return false;
			const std::size_t size = (static_cast<std::size_t>(reader.pending[0]) << 8) | reader.pending[1];
			if (reader.pending.size() < 4 + size) return false;
			const auto rawType = static_cast<uint16_t>((static_cast<uint16_t>(reader.pending[2]) << 8) | reader.pending[3]);
			body.assign(reinterpret_cast<const char*>(reader.pending.data() + 4), size);
			type = static_cast<MessageType>(rawType);
			reader.pending.erase(reader.pending.begin(), reader.pending.begin() + 4 + size);
			return true;
		};
		if (extract()) return PollStatus::Message;
		std::vector<uint8_t> bytes;
		const auto status = connection->ReceiveAvailable(bytes);
		if (status == QFE::NETWORK::ReceiveStatus::Pending) return PollStatus::Pending;
		if (status == QFE::NETWORK::ReceiveStatus::Closed) return reader.pending.empty() ? PollStatus::Closed : PollStatus::Error;
		if (status != QFE::NETWORK::ReceiveStatus::Data) return PollStatus::Error;
		reader.pending.insert(reader.pending.end(), bytes.begin(), bytes.end());
		return extract() ? PollStatus::Message : PollStatus::Pending;
	}

	bool CreateNetworkHeader(std::size_t bodySize, MessageType type, std::array<uint8_t, 4>& header) {
		if (bodySize > (std::numeric_limits<uint16_t>::max)()) return false;
		const auto length = static_cast<uint16_t>(bodySize);
		const auto rawType = static_cast<uint16_t>(type);
		// 異なるCPU間でも解釈が一致するよう、上位バイトから並べる。
		header = { static_cast<uint8_t>(length >> 8), static_cast<uint8_t>(length),
			static_cast<uint8_t>(rawType >> 8), static_cast<uint8_t>(rawType) };
		return true;
	}

	bool CreateNetworkMessage(MessageType type, const std::string& body, std::vector<uint8_t>& message) {
		std::array<uint8_t, 4> header{};
		if (!CreateNetworkHeader(body.size(), type, header)) return false;
		std::vector<uint8_t> result;
		result.reserve(header.size() + body.size());
		result.insert(result.end(), header.begin(), header.end());
		// 文字コード変換は行わず、文字列が保持するバイトをそのまま送る。
		for (const char byte : body) result.push_back(static_cast<uint8_t>(byte));
		message = std::move(result);
		return true;
	}

	bool SendString(QFE::NETWORK::INetworkCore* core, QFE::NETWORK::ConnectHandle handle,
		const std::string& body) {
		return SendNetworkMessage(core, handle, MessageType::Message, body);
	}

	bool SendNetworkMessage(QFE::NETWORK::INetworkCore* core, QFE::NETWORK::ConnectHandle handle,
		MessageType type, const std::string& body) {
		if (!core) return false;
		auto* connection = core->FindConnection(handle);
		if (!connection) return false;
		std::vector<uint8_t> message;
		if (!CreateNetworkMessage(type, body, message)) return false;
		// 送信処理が分割される場合もあるため、同じ接続の他の送信と並行させない。
		return connection->SendBlocking(message.data(), message.size());
	}

	bool ReceiveMessage(QFE::NETWORK::INetworkCore* core, QFE::NETWORK::ConnectHandle handle,
		MessageType& type, std::string& body) {
		if (!core) return false;
		auto* connection = core->FindConnection(handle);
		if (!connection) return false;
		std::array<uint8_t, 4> header{};
		if (!connection->ReceiveBlocking(header.data(), header.size())) return false;
		uint16_t length = 0;
		uint16_t rawType = 0;
		// ヘッダーは本文長、種別の順に各16ビットを上位バイトから格納する。
		length = static_cast<uint16_t>((static_cast<uint16_t>(header[0]) << 8) | header[1]);
		rawType = static_cast<uint16_t>((static_cast<uint16_t>(header[2]) << 8) | header[3]);
		std::string received(length, '\0');
		if (!connection->ReceiveBlocking(reinterpret_cast<uint8_t*>(received.data()), received.size())) return false;
		// 途中受信に失敗した場合は、呼び出し元の出力を変更しない。
		type = static_cast<MessageType>(rawType);
		body = std::move(received);
		return true;
	}
}
