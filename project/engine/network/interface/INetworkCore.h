#pragma once
#include "info/SupportedOs.h"
#include "network/Connecthandle.h"
#include <string>

namespace QFE::NETWORK {
	// 受け入れ待ちと失敗を区別し、画面更新を止めずに接続を確認する。
	enum class AcceptStatus { Accepted, Pending, Error };
	class ITransceiver;
	class INetworkCore {
	public:
		virtual ~INetworkCore() = default;

		/// @brief ネットワークコアのサポートOSを取得します.
		virtual QFE::INFO::SupportedOs GetSupportedOs() const = 0;
		// 接続の所有はネットワークコアに残し、呼び出し元にはハンドルだけを返す。
		virtual ConnectHandle Connect(const std::string& host, const std::string& port) = 0;

		/// @brief 指定されたハンドルに対応する接続を検索します。
		/// @param handle 検索する接続のハンドル。
		/// @return 対応する接続を表す ITransceiver オブジェクトへのポインター。接続が見つからない場合は nullptr を返します。
		virtual ITransceiver* FindConnection(ConnectHandle handle) = 0;

		/// @brief 接続を切断します。
		/// @param handle 切断する接続を識別するハンドル。
		/// @return 接続の切断に成功した場合は true、それ以外の場合は false。
		virtual bool Disconnect(ConnectHandle handle) = 0;
		virtual bool ListenLocal(uint16_t& port) = 0;
		virtual AcceptStatus Accept(ConnectHandle& handle) = 0;
		virtual void StopListening() = 0;
	};
}
