#pragma once
#include "network/INetworkCore.h"

namespace QFE::NETWORK {
	class WindowsNetworkCore : public INetworkCore {
	public:
		/// @brief ネットワークコアのサポートOSを取得します.
		QFE::INFO::SupportedOs GetSupportedOs() const override;

		/// @brief ネットワークコアを初期化します.
		bool Initialize();
	};
}