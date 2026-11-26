#pragma once
#include "framework/network/MessageType.h"
#include "network/interface/INetworkCore.h"
#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace QFE::FRAMEWORK::NETWORK {
	using AcceptStatus = QFE::NETWORK::AcceptStatus;
	enum class PollStatus { Message, Pending, Closed, Error };

	struct MessageReader {
		// TCPの分割受信を次のフレームへ持ち越す。
		std::vector<uint8_t> pending;
	};
	/// @brief ネットワーク機能を生成・初期化する。失敗時はnullptrを返す。
	/// @returns 初期化済みのネットワークコアのユニークポインタ。失敗時はnullptr。
	std::unique_ptr<QFE::NETWORK::INetworkCore> CreateNetworkCore();

	/// @brief 残っている接続を閉じ、ネットワーク機能を終了する。
	/// @param core 終了するネットワークコアのポインタ。
	/// @returns 終了処理が成功した場合はtrue、それ以外の場合
	bool ShutdownNetworkCore(QFE::NETWORK::INetworkCore* core);

	/// @brief TCPで接続する。失敗時は無効なハンドルを返す。完了まで待機する。
	/// @param core 接続を行うネットワークコアのポインタ。
	/// @param host 接続先のホスト名またはIPアドレス。
	/// @param port 接続先のポート番号。
	/// @returns 接続ハンドル。失敗時は無効なハンドル。
	QFE::NETWORK::ConnectHandle Connect(QFE::NETWORK::INetworkCore* core,
		const std::string& host, const std::string& port);

	/// @brief 接続を閉じ、ハンドルを無効にする。
	/// @param core 接続を閉じるネットワークコアのポインタ。
	/// @param handle 閉じる接続のハンドル。
	/// @returns 接続の切断に成功した場合はtrue、それ以外の場合はfalse。
	bool Disconnect(QFE::NETWORK::INetworkCore* core, QFE::NETWORK::ConnectHandle handle);

	/// @brief ローカル専用の空きポートで待ち受けを開始する。
	bool ListenLocal(QFE::NETWORK::INetworkCore* core, uint16_t& port);

	/// @brief 接続を待たずに受け入れを確認する。
	AcceptStatus Accept(QFE::NETWORK::INetworkCore* core, QFE::NETWORK::ConnectHandle& handle);

	/// @brief ネットワークコアのリスニングを停止します。
	/// @param core ネットワークコアへのポインター。
	void StopListening(QFE::NETWORK::INetworkCore* core);
	/// @brief 受信済みのバイトを組み立て、待機せず1件のメッセージを取り出す。
	PollStatus PollMessage(QFE::NETWORK::INetworkCore* core, QFE::NETWORK::ConnectHandle handle,
		MessageReader& reader, MessageType& type, std::string& body);

	/// @brief 本文長と種別から、通信に使用する4バイトのヘッダーを作る。
	/// @details 本文長はバイト数。65535バイトを超える場合はfalseを返し、出力を変更しない。
	bool CreateNetworkHeader(std::size_t bodySize, MessageType type, std::array<uint8_t, 4>& header);

	/// @brief ヘッダーと本文を結合し、送信用のバイト列を作る。
	/// @details 作成に失敗した場合は出力を変更しない。
	bool CreateNetworkMessage(MessageType type, const std::string& body, std::vector<uint8_t>& message);

	/// @brief 文字列をMessage種別で送信する。送信が完了するまで待機する。
	bool SendString(QFE::NETWORK::INetworkCore* core, QFE::NETWORK::ConnectHandle handle,
		const std::string& body);

	/// @brief 種別とUTF-8等のバイト列を送る。完了まで呼び出し元のスレッドを待たせる。
	/// @param core メッセージを送信するネットワークコアのポインタ。
	/// @param handle メッセージを送信する接続のハンドル。
	/// @param type 送信するメッセージの種別。
	/// @param body 送信するメッセージの本文。
	/// @returns メッセージの送信に成功した場合はtrue、それ以外の場合はfalse。
	bool SendNetworkMessage(QFE::NETWORK::INetworkCore* core, QFE::NETWORK::ConnectHandle handle,
		MessageType type, const std::string& body);

	/// @brief 1件のメッセージを受け取る。全データが届くまでスレッドを待たせる。
	/// @param core メッセージを受信するネットワークコアのポインタ。
	/// @param handle メッセージを受信する接続のハンドル。
	/// @param type 受信したメッセージの種別。
	/// @param body 受信したメッセージの本文。
	/// @returns メッセージの受信に成功した場合はtrue、それ以外の場合はfalse。
	bool ReceiveMessage(QFE::NETWORK::INetworkCore* core, QFE::NETWORK::ConnectHandle handle,
		MessageType& type, std::string& body);
}
