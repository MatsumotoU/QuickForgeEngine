#pragma once
namespace QFE {
	class IEntityManager;
}
namespace QFE::INPUT {
	class InputInterface;
}

namespace QFE::SCRIPT {
	/// @brief スクリプトからエンジンの機能にアクセスするためのコンテキスト
	class EngineContextForScript {
	public:
		/// @brief コンストラクタ
		EngineContextForScript(QFE::IEntityManager* entityManager, QFE::INPUT::InputInterface* inputInterface);
		QFE::IEntityManager* entityManager_;
		QFE::INPUT::InputInterface* inputInterface_;
	};
}