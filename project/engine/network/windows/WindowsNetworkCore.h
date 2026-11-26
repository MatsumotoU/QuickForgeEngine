#pragma once
#include "network/interface/INetworkCore.h"
#include "WindowsSinglePeerTransceiver.h"
#include <memory>
#include <vector>

namespace QFE::NETWORK {

	/// @brief 複数の接続を管理するネットワークコア。Windows専用。
	class WindowsNetworkCore : public INetworkCore {
	public:
		~WindowsNetworkCore() override;
		/// @brief ネットワークコアのサポートOSを取得します.
		QFE::INFO::SupportedOs GetSupportedOs() const override;

		/// @brief ネットワークコアを初期化します.
		bool Initialize();

		/// @brief 接続を確立します。成功した場合は接続ハンドルを返し、失敗した場合は無効なハンドルを返します。
		/// @param host 接続先のホスト名またはIPアドレス。
		/// @param port 接続先のポート番号。
		/// @return 接続ハンドル。失敗時は無効なハンドル。
		ConnectHandle Connect(const std::string& host, const std::string& port) override;

		/// @brief 接続ハンドルに対応する接続を検索します。
		/// @param handle 検索する接続のハンドル。
		/// @return 対応するトランシーバーへのポインター。接続が見つからない場合の値はnullptrです。
		ITransceiver* FindConnection(ConnectHandle handle) override;

		/// @brief 接続を切断します。
		/// @param handle 切断する接続のハンドル。
		/// @return 接続の切断に成功した場合はtrue、それ以外の場合はfalse。
		bool Disconnect(ConnectHandle handle) override;
		bool ListenLocal(uint16_t& port) override;
		AcceptStatus Accept(ConnectHandle& handle) override;
		void StopListening() override;

		/// @brief ネットワークコアを終了します。
		/// @return 終了処理が成功した場合はtrue、それ以外の場合はfalse。
		bool Finalize();

	private:
		ConnectHandle StoreConnection(std::unique_ptr<WindowsSinglePeerTransceiver> connection);
		struct Slot {
			std::unique_ptr<WindowsSinglePeerTransceiver> connection;
			uint32_t generation = 0;
		};
		std::vector<Slot> connections_;
		bool initialized_ = false;
		SOCKET listener_ = INVALID_SOCKET;
	};
}
