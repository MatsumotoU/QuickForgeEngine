#pragma once
#include "info/SupportedOs.h"

namespace QFE::NETWORK {
	class INetworkCore {
	public:
		virtual ~INetworkCore() = default;

		/// @brief ネットワークコアのサポートOSを取得します.
		virtual QFE::INFO::SupportedOs GetSupportedOs() const = 0;
	};
}